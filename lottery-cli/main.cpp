#include <algorithm>
#include <cassert>
#include <cstdint>
#include <sstream>

#include <fmt/format.h>

#include <restinio/all.hpp>

#include "crypto/block/block.h"
#include "crypto/vm/boc.h"
#include "crypto/vm/dict.h"

#include "td/actor/ActorOwn.h"
#include "td/actor/common.h"
#include "td/utils/filesystem.h"
#include <td/utils/OptionsParser.h>
#include "td/utils/Status.h"

#include "tonlib/TonlibCallback.h"
#include "tonlib/TonlibClient.h"

#include "SpecEntry.h"

namespace tonlib_api = ton::tonlib_api;

td::actor::Scheduler* g_scheduler_ptr{nullptr};

using Spec = std::vector<TlSpecEntry*>;

// aka Lottery smc
class TonlibConsumer : public td::actor::Actor
{
public:
    struct Options
    {
      std::string config;
      std::string name;
      bool use_callbacks_for_network{false};
      bool ignore_cache{false};
      std::string smc_addr;
    };

    explicit TonlibConsumer(Options options) : options_{std::move(options)}
    {
    }

    void get_(std::string method_name, Spec in_spec, const Spec& out_spec, td::Promise<std::string> promise)
    {
        using tonlib_api::make_object;

        std::vector<tonlib_api::object_ptr<tonlib_api::tvm_StackEntry>> stack;
        for (const TlSpecEntry* se : in_spec) {
            // TODO:
            auto s0 = make_object<tonlib_api::tvm_stackEntryNumber>(make_object<tonlib_api::tvm_numberDecimal>(se->name()));
            stack.push_back(std::move(s0));
        }

        run_method(std::move(method_name), std::move(stack),
                   promise.send_closure(td::actor::actor_id(this), &TonlibConsumer::got_, out_spec));
    }

    void got_(const Spec& out_spec, tonlib_api::object_ptr<tonlib_api::smc_runResult> info, td::Promise<std::string> promise)
    {
        if (info->exit_code_ != 0) {
            promise.set_error(td::Status::Error(1, "err"));
            return;
        }

        std::vector<std::string> pre_out(out_spec.size());

        std::transform(out_spec.begin(), out_spec.end(), info->stack_.begin(), pre_out.begin(),
                [](const TlSpecEntry* a, const tonlib_api::object_ptr<tonlib_api::tvm_StackEntry>& b) {
                    return a->process(*b);
                }
        );

        std::stringstream ss;
        ss << '{';
        bool first{true};
        for (const auto& s : pre_out) {
            if (!first) {
                ss << ',';
            } else {
                first = false;
            }

            ss << s;
        }
        ss << '}';
        auto res = ss.str();

        promise.set_value(std::move(res));
    }

    void load_smc(td::Promise<tonlib_api::object_ptr<tonlib_api::smc_info>> promise)
    {
      using tonlib_api::make_object;

      auto smc_addr = make_object<tonlib_api::accountAddress>(options_.smc_addr);
      auto load_smc_q = make_object<tonlib_api::smc_load>(std::move(smc_addr));
      send_query(std::move(load_smc_q), std::move(promise));
    }

    void run_method(std::string method_name, std::vector<tonlib_api::object_ptr<tonlib_api::tvm_StackEntry>> stack,
        td::Promise<tonlib_api::object_ptr<tonlib_api::smc_runResult>> promise)
    {
      using tonlib_api::make_object;

      auto method = make_object<tonlib_api::smc_methodIdName>(std::move(method_name));
      auto to_run = make_object<tonlib_api::smc_runGetMethod>(0 /*fixme*/, std::move(method), std::move(stack));

      auto smc_addr = make_object<tonlib_api::accountAddress>(options_.smc_addr);
      auto load_smc_q = make_object<tonlib_api::smc_load>(std::move(smc_addr));
      send_query(std::move(load_smc_q),
          promise.send_closure(td::actor::actor_id(this), &TonlibConsumer::run_method_2, std::move(to_run)));
    }

    void run_method_2(tonlib_api::object_ptr<tonlib_api::smc_runGetMethod> to_run,
            tonlib_api::object_ptr<tonlib_api::smc_info> info,
            td::Promise<tonlib_api::object_ptr<tonlib_api::smc_runResult>> promise)
    {
      to_run->id_ = info->id_;
      send_query(std::move(to_run), std::move(promise));
    }

private:
    Options options_;
    td::actor::ActorOwn<tonlib::TonlibClient> tonlib_;
    std::uint64_t next_query_id_{1};
    std::map<std::uint64_t, td::Promise<tonlib_api::object_ptr<tonlib_api::Object>>> query_handlers_;

    void start_up() override
    {
      class TonlibCb : public tonlib::TonlibCallback
      {
      public:
          explicit TonlibCb(td::actor::ActorShared<TonlibConsumer> id) : id_{std::move(id)} {}

          void on_result(std::uint64_t id, tonlib_api::object_ptr<tonlib_api::Object> result) override
          {
            td::actor::send_closure(id_, &TonlibConsumer::on_tonlib_result, id, std::move(result));
          }

          void on_error(std::uint64_t id, tonlib_api::object_ptr<tonlib_api::error> error) override
          {
            td::actor::send_closure(id_, &TonlibConsumer::on_tonlib_error, id, std::move(error));
          }
      private:
          td::actor::ActorShared<TonlibConsumer> id_;
      };

      tonlib_ = td::actor::create_actor<tonlib::TonlibClient>("Tonlib", td::make_unique<TonlibCb>(td::actor::actor_shared(this)));

      init_tonlib();

      sync_tonlib();
    }

    void init_tonlib()
    {
      using tonlib_api::make_object;
      auto config = make_object<tonlib_api::config>(options_.config, options_.name, options_.use_callbacks_for_network,
          options_.ignore_cache);
      auto ks_type = make_object<tonlib_api::keyStoreTypeInMemory>();
      auto options = make_object<tonlib_api::options>(std::move(config), std::move(ks_type));
      auto init_query = make_object<tonlib_api::init>(std::move(options));
      send_query(std::move(init_query), [](auto r_ok) {});
    }

    void sync_tonlib()
    {
      using tonlib_api::make_object;
      send_query(make_object<tonlib_api::sync>(), [](auto r_ok) {});
    }

    template <typename QueryT>
    void send_query(tonlib_api::object_ptr<QueryT> query, td::Promise<typename QueryT::ReturnType> promise)
    {
      auto query_id = next_query_id_++;
      tonlib_api::object_ptr<tonlib_api::Function> func = std::move(query);
      td::actor::send_closure(tonlib_, &tonlib::TonlibClient::request, query_id, std::move(func));
      query_handlers_[query_id] =
          [promise = std::move(promise)](td::Result<tonlib_api::object_ptr<tonlib_api::Object>> r_obj) mutable {
            if (r_obj.is_error()) {
              return promise.set_error(r_obj.move_as_error());
            }
            promise.set_value(ton::move_tl_object_as<typename QueryT::ReturnType::element_type>(r_obj.move_as_ok()));
          };
    }

    void on_tonlib_result(std::uint64_t id, tonlib_api::object_ptr<tonlib_api::Object> result)
    {
      auto it = query_handlers_.find(id);
      if (it == query_handlers_.end()) {
        return;
      }
      auto promise = std::move(it->second);
      query_handlers_.erase(it);
      promise.set_value(std::move(result));
    }

    void on_tonlib_error(std::uint64_t id, tonlib_api::object_ptr<tonlib_api::error> error)
    {
      auto it = query_handlers_.find(id);
      if (it == query_handlers_.end()) {
        return;
      }
      auto promise = std::move(it->second);
      query_handlers_.erase(it);
      promise.set_error(td::Status::Error(error->code_, error->message_));
    }
};

using router_t = restinio::router::express_router_t<>;

template <typename RESP>
RESP init_resp(RESP resp)
{
    resp.append_header(restinio::http_field::server, "RESTinio sample server /v.0.2");
    resp.append_header(restinio::http_field::access_control_allow_origin, "*");
    resp.append_header_date_field();

    return resp;
}

auto create_router(td::actor::ActorOwn<TonlibConsumer>& tonClient)
{
  assert(g_scheduler_ptr);

  auto router = std::make_unique<router_t>();

  auto register_route = [&tonClient, &router](std::string path, Spec in_spec, Spec out_spec) {
      std::string args;
      if (!in_spec.empty()) {
          args = R"((\?.*)?)";
      }

      auto handler = [&tonClient, path, in_spec = std::move(in_spec), out_spec = std::move(out_spec)](auto req, auto params) {
          Spec tmp_in_spec;
          if (!in_spec.empty()) {
              const auto qp = restinio::parse_query(req->header().query());
              for (const TlSpecEntry *se : in_spec) {
                  if (qp.has(se->name())) {
                      auto p = qp[se->name()];

                      // TODO: transfrom?
                      block::StdAddress a;
                      if (!a.parse_addr(td::Slice{p.data(), p.size()})) {
                          return restinio::request_rejected();
                      }
                      tmp_in_spec.emplace_back(new TlSpecEntry_Number{std::to_string(a.workchain)});  // TODO: memleak
                      auto x = td::bits_to_refint(a.addr.cbits(), a.addr.size(), false);
                      tmp_in_spec.emplace_back(new TlSpecEntry_Number{dec_string(x)});  // TODO: memleak
                  }
              }
          }

          g_scheduler_ptr->run_in_context_external([&tonClient, path, req, tmp_in_spec = std::move(tmp_in_spec), &out_spec]() mutable {
              auto closure = [req](td::Result<std::string> res) {
                  if (res.is_error()) {
                      init_resp(req->create_response(restinio::status_internal_server_error()))
                              .done();
                      return;
                  }

                  init_resp(req->create_response())
                          .append_header(restinio::http_field::content_type, "text/json; charset=utf-8")
                          .set_body(res.move_as_ok())
                          .done();
              };

              td::actor::send_closure(tonClient, &TonlibConsumer::get_, path, std::move(tmp_in_spec), out_spec, std::move(closure));
          });

          return restinio::request_accepted();
      };

      router->http_get("/" + path + args, std::move(handler));
  };

    {
        Spec in_spec;

        Spec out_spec;
        out_spec.emplace_back(new TlSpecEntry_Number{"prize_fund"});  // TODO: memleak

        register_route("prize_fund", std::move(in_spec), std::move(out_spec));
    }

    {
        Spec in_spec;

        Spec out_spec;
        auto key_wc = new TvmSpecEntry_Int{"wc", 8};
        auto key_addr = new TvmSpecEntry_Int256{"addr", 256, false};
        auto key = new TvmSpecEntry_User{"addr", {key_wc, key_addr}};
        auto num1 = new TvmSpecEntry_UInt{"n1", 8};
        auto num2 = new TvmSpecEntry_UInt{"n2", 8};
        auto num3 = new TvmSpecEntry_UInt{"n3", 8};
        auto value = new TvmSpecEntry_User{"nums", {num1, num2, num3}};
        out_spec.emplace_back(new TlSpecEntry_Dict{"participants", key, value});  // TODO: memleak

        register_route("participants", std::move(in_spec), std::move(out_spec));
    }

    {
        Spec in_spec;

        Spec out_spec;
        out_spec.emplace_back(new TlSpecEntry_List{"lucky_nums"});  // TODO: memleak

        register_route("lucky_nums", std::move(in_spec), std::move(out_spec));
    }

    {
        Spec in_spec;

        Spec out_spec;
        out_spec.emplace_back(new TlSpecEntry_Number{"p1"});  // TODO: memleak
        out_spec.emplace_back(new TlSpecEntry_Number{"p2"});  // TODO: memleak
        out_spec.emplace_back(new TlSpecEntry_Number{"p3"});  // TODO: memleak

        register_route("prizes", std::move(in_spec), std::move(out_spec));
    }

    {
        Spec in_spec;
        in_spec.emplace_back(new TlSpecEntry_Number{"addr"});  // TODO: memleak

        Spec out_spec;
        out_spec.emplace_back(new TlSpecEntry_Number{"prize"});  // TODO: memleak

        register_route("is_winner", std::move(in_spec), std::move(out_spec));
    }

  /*router->non_matched_request_handler([](auto req) {
    req->create_response(restinio::status_not_found)
      .append_header_date_field()
      .connection_close()
      .done();
    
    return restinio::request_rejected();
  });*/

  return router;
}

//

int main(int argc, char* argv[])
{
    TonlibConsumer::Options options;

    td::OptionsParser p;
    p.set_description("lottery web server");
    p.add_option('C', "config-force", "set lite server config, drop config related blockchain cache", [&](td::Slice arg) {
        TRY_RESULT(data, td::read_file_str(arg.str()));
        options.config = std::move(data);
        options.ignore_cache = true;
        return td::Status::OK();
    });
    p.add_option('c', "config", "set lite server config", [&](td::Slice arg) {
        TRY_RESULT(data, td::read_file_str(arg.str()));
        options.config = std::move(data);
        return td::Status::OK();
    });
    p.add_option('a', "address", "set lottery smc address", [&](td::Slice arg) {
        options.smc_addr = arg.str();
        return td::Status::OK();
    });

    auto S = p.run(argc, argv);
    if (S.is_error()) {
        std::cerr << S.move_as_error().message().str() << std::endl;
        std::_Exit(2);
    }

    td::actor::Scheduler scheduler{{2}};
    g_scheduler_ptr = &scheduler;
    td::thread scheduler_thread;
    td::actor::ActorOwn<TonlibConsumer> lottery;

    scheduler.run_in_context([&] {
        lottery = td::actor::create_actor<TonlibConsumer>("TonlibConsumer", options);
    });

    scheduler_thread = td::thread([&] {
        scheduler.run();
    });

    using server_traits_t = restinio::traits_t<
        restinio::asio_timer_manager_t,
        restinio::shared_ostream_logger_t,
        router_t
    >;

    restinio::run(
        restinio::on_this_thread<server_traits_t>()
            .port(8080)
            .address("0.0.0.0")
            .request_handler(create_router(lottery))
    );

    scheduler.run_in_context_external([&] {
      lottery.reset();
    });
    // TODO: wait
    scheduler.run_in_context_external([] {
      td::actor::SchedulerContext::get()->stop();
    });
    scheduler_thread.join();
}
