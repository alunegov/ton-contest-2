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

namespace tonlib_api = ton::tonlib_api;

td::actor::Scheduler* g_scheduler_ptr{nullptr};

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

    void get_prize_fund(td::Promise<std::int64_t> promise)
    {
      std::vector<tonlib_api::object_ptr<tonlib_api::tvm_StackEntry>> stack;

      run_method("prize_fund", std::move(stack),
          promise.send_closure(td::actor::actor_id(this), &TonlibConsumer::got_prize_fund));
    }

    void got_prize_fund(tonlib_api::object_ptr<tonlib_api::smc_runResult> info, td::Promise<std::int64_t> promise)
    {
      auto& entry = static_cast<tonlib_api::tvm_stackEntryNumber&>(*info->stack_[0]);
      auto val = std::stoll(entry.number_->number_);
      promise.set_value(val);
    }

    struct Participant
    {
      std::string addr_;
      std::array<std::uint8_t, 3> nums_;
      Participant(std::string addr, std::array<std::uint8_t, 3> nums) : addr_{std::move(addr)}, nums_{std::move(nums)}
      {}
    };

    void get_participants(td::Promise<std::vector<Participant>> promise)
    {
      std::vector<tonlib_api::object_ptr<tonlib_api::tvm_StackEntry>> stack;

      run_method("participants", std::move(stack),
          promise.send_closure(td::actor::actor_id(this), &TonlibConsumer::got_participants));
    }

    void got_participants(tonlib_api::object_ptr<tonlib_api::smc_runResult> info, td::Promise<std::vector<Participant>> promise)
    {
      // TODO: empty dict - null()
      auto& entry = static_cast<tonlib_api::tvm_stackEntryCell&>(*info->stack_[0]);
      auto cell = vm::std_boc_deserialize(entry.cell_->bytes_);
      auto dict = vm::Dictionary{cell.move_as_ok(), 8 + 256};
      std::vector<Participant> val;
      for (auto it : dict.range(false, false)) {
        auto k = it.first.to_hex(8 + 256);
        auto z = dict.key_as_integer(it.first, false);
        auto v = it.second->clone();
        auto n1 = static_cast<uint8_t>(v.fetch_ulong(8));
        auto n2 = static_cast<uint8_t>(v.fetch_ulong(8));
        auto n3 = static_cast<uint8_t>(v.fetch_ulong(8));
        assert(v.empty());
        val.emplace_back(k, std::array<std::uint8_t, 3>{n1, n2, n3});
      }
      promise.set_value(std::move(val));
    }

    void get_lucky_nums(td::Promise<std::vector<std::uint8_t>> promise)
    {
      std::vector<tonlib_api::object_ptr<tonlib_api::tvm_StackEntry>> stack;

      run_method("lucky_nums", std::move(stack),
          promise.send_closure(td::actor::actor_id(this), &TonlibConsumer::got_lucky_nums));
    }

    void got_lucky_nums(tonlib_api::object_ptr<tonlib_api::smc_runResult> info, td::Promise<std::vector<std::uint8_t>> promise)
    {
      // TODO: empty tuple - nil
      auto& entry = static_cast<tonlib_api::tvm_stackEntryList&>(*info->stack_[0]);
      std::vector<std::uint8_t> val;
      val.reserve(entry.list_->elements_.size());
      for (auto& el : entry.list_->elements_) {
        auto& elv = static_cast<tonlib_api::tvm_stackEntryNumber&>(*el);
        val.push_back(std::stoul(elv.number_->number_));
      }
      promise.set_value(std::move(val));
    }

    void get_prizes(td::Promise<std::vector<std::int64_t>> promise)
    {
      std::vector<tonlib_api::object_ptr<tonlib_api::tvm_StackEntry>> stack;

      run_method("prizes", std::move(stack),
          promise.send_closure(td::actor::actor_id(this), &TonlibConsumer::got_prizes));
    }

    void got_prizes(tonlib_api::object_ptr<tonlib_api::smc_runResult> info, td::Promise<std::vector<std::int64_t>> promise)
    {
      std::vector<std::int64_t> val;
      val.reserve(info->stack_.size());
      for (auto& el : info->stack_) {
        auto& elv = static_cast<tonlib_api::tvm_stackEntryNumber&>(*el);
        val.push_back(std::stoll(elv.number_->number_));
      }
      promise.set_value(std::move(val));
    }

    void get_is_winner(std::string addr, td::Promise<std::int64_t> promise)
    {
      using tonlib_api::make_object;

      block::StdAddress a;
      if (!a.parse_addr(td::Slice{addr})) {
        promise.set_error(td::Status::Error(1112, "qqq"));
      }

      std::vector<tonlib_api::object_ptr<tonlib_api::tvm_StackEntry>> stack;

      auto s0 = make_object<tonlib_api::tvm_stackEntryNumber>(make_object<tonlib_api::tvm_numberDecimal>("0"/*a.workchain*/));
      stack.push_back(std::move(s0));

      auto num = td::RefInt256{true};
      auto& x = num.unique_write();
      x.parse_dec(addr.data(), addr.size());
      auto s1 = make_object<tonlib_api::tvm_stackEntryNumber>(make_object<tonlib_api::tvm_numberDecimal>(dec_string(num)));
      stack.push_back(std::move(s1));

      run_method("is_winner", std::move(stack),
              promise.send_closure(td::actor::actor_id(this), &TonlibConsumer::got_is_winner));
    }

    void got_is_winner(tonlib_api::object_ptr<tonlib_api::smc_runResult> info, td::Promise<std::int64_t> promise)
    {
      if (info->exit_code_ != 0) {
        promise.set_error(td::Status::Error(1111, "www"));
      }
      auto& entry = static_cast<tonlib_api::tvm_stackEntryNumber&>(*info->stack_[0]);
      auto val = std::stoll(entry.number_->number_);
      promise.set_value(val);
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

std::ostream& operator<<(std::ostream& stream, TonlibConsumer::Participant participant) {
  return stream << "{\"addr\":\"" + participant.addr_ << "\",\"nums\":["
      << std::to_string(participant.nums_[0]) << ',' << std::to_string(participant.nums_[1]) << ','
      << std::to_string(participant.nums_[2]) << "]}";
}

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

  router->http_get("/prize_fund", [&](auto req, auto) {
    g_scheduler_ptr->run_in_context_external([&] {
      td::actor::send_closure(tonClient, &TonlibConsumer::get_prize_fund,
          [req](td::Result<std::int64_t> res) {
              if (res.is_error()) {
                  init_resp(req->create_response(restinio::status_internal_server_error()))
                          .done();
                  return;
              }
              
              auto b = fmt::format(R"({{"prize_fund":{}}})", res.ok());

            init_resp(req->create_response())
              .append_header(restinio::http_field::content_type, "text/json; charset=utf-8")
              .set_body(b)
              .done();
          });
    });

    return restinio::request_accepted();
  });

  router->http_get("/participants", [&](auto req, auto) {
    g_scheduler_ptr->run_in_context_external([&] {
      td::actor::send_closure(tonClient, &TonlibConsumer::get_participants,
          [req](td::Result<std::vector<TonlibConsumer::Participant>> res) {
              if (res.is_error()) {
                  init_resp(req->create_response(restinio::status_internal_server_error()))
                          .done();
                  return;
              }
              
              auto r = res.move_as_ok();

              std::stringstream ss;
              ss << R"({"participants":[)";
              for (size_t i = 0; i < r.size() - 1; ++i) {
                ss << r[i] << ',';
              }
              ss << r[r.size() - 1];
              ss << R"(]})";

              auto b = ss.str();

            init_resp(req->create_response())
              .append_header(restinio::http_field::content_type, "text/json; charset=utf-8")
              .set_body(b)
              .done();
          });
    });

    return restinio::request_accepted();
  });

  router->http_get("/lucky_nums", [&](auto req, auto) {
    g_scheduler_ptr->run_in_context_external([&] {
      td::actor::send_closure(tonClient, &TonlibConsumer::get_lucky_nums,
          [req](td::Result<std::vector<std::uint8_t>> res) {
              if (res.is_error()) {
                  init_resp(req->create_response(restinio::status_internal_server_error()))
                          .done();
                  return;
              }

              auto r = res.move_as_ok();

              std::stringstream ss;
              ss << R"({"lucky_nums":[)";
              for (size_t i = 0; i < r.size() - 1; ++i) {
                ss << std::to_string(r[i]) << ',';
              }
              ss << std::to_string(r[r.size() - 1]);
              ss << R"(]})";

              auto b = ss.str();

            init_resp(req->create_response())
              .append_header(restinio::http_field::content_type, "text/json; charset=utf-8")
              .set_body(b)
              .done();
          });
    });

    return restinio::request_accepted();
  });

  router->http_get("/prizes", [&](auto req, auto) {
    g_scheduler_ptr->run_in_context_external([&] {
      td::actor::send_closure(tonClient, &TonlibConsumer::get_prizes,
          [req](td::Result<std::vector<std::int64_t>> res) {
              if (res.is_error()) {
                  init_resp(req->create_response(restinio::status_internal_server_error()))
                          .done();
                  return;
              }

              auto r = res.move_as_ok();

              std::stringstream ss;
              ss << R"({"prizes":[)";
              for (size_t i = 0; i < r.size() - 1; ++i) {
                ss << r[i] << ',';
              }
              ss << r[r.size() - 1];
              ss << R"(]})";

              auto b = ss.str();

            init_resp(req->create_response())
              .append_header(restinio::http_field::content_type, "text/json; charset=utf-8")
              .set_body(b)
              .done();
          });
    });

    return restinio::request_accepted();
  });

  router->http_get("/is_winner/:addr", [&](auto req, auto params) {
    auto addr = restinio::cast_to<std::string>(params["addr"]);

    g_scheduler_ptr->run_in_context_external([&] {
      td::actor::send_closure(tonClient, &TonlibConsumer::get_is_winner, std::move(addr),
          [req](td::Result<std::int64_t> res) {
            if (res.is_error()) {
                init_resp(req->create_response(restinio::status_internal_server_error()))
                        .done();
                return;
            }

            auto b = fmt::format(R"({{"prize":{}}})", res.ok());

            init_resp(req->create_response())
              .append_header(restinio::http_field::content_type, "text/json; charset=utf-8")
              .set_body(b)
              .done();
          });
    });

    return restinio::request_accepted();
  });

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
            .address("localhost")
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
