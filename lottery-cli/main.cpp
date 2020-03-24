#include <cassert>
#include <cstdint>
#include <sstream>

#include <fmt/format.h>

#include <restinio/all.hpp>

#include "td/actor/ActorOwn.h"
#include "td/actor/common.h"
#include "td/utils/filesystem.h"
#include <td/utils/OptionsParser.h>
#include "td/utils/Status.h"

#include "tonlib/TonlibCallback.h"
#include "tonlib/TonlibClient.h"

namespace tonlib_api = ton::tonlib_api;

td::actor::Scheduler* g_scheduler_ptr{nullptr};

struct InMemoryStore
{
  std::int64_t prize_fund{0};
  std::size_t participants_count{0};
  std::vector<std::uint8_t> lucky_nums;
  std::vector<std::int64_t> prizes;
};

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

    struct CurrentRound
    {
      std::int64_t prize_fund;
      std::int64_t participants_count;
    };

    void current_round(td::Promise<CurrentRound> promise)
    {
      load_smc(promise.send_closure(td::actor::actor_id(this), &TonlibConsumer::got_current_round));
    }

    void got_current_round(tonlib_api::object_ptr<tonlib_api::smc_info> info, td::Promise<CurrentRound> promise)
    {
      // SmartContract smc{state};
      // smc.run_getmethod('prize_fund');
      // smc.run_getmethod('participants');

      promise.set_value({0, 0});
    }

    struct PrevRound
    {
      std::vector<std::uint8_t> lucky_nums;
      std::vector<std::int64_t> prizes;
    };

    void prev_round(td::Promise<PrevRound> promise)
    {
      load_smc(promise.send_closure(td::actor::actor_id(this), &TonlibConsumer::got_prev_round));
    }

    void got_prev_round(tonlib_api::object_ptr<tonlib_api::smc_info> info, td::Promise<PrevRound> promise)
    {
      using tonlib_api::make_object;

      auto get_state_q = make_object<tonlib_api::smc_getState>(info->id_);
      send_query(std::move(get_state_q),
          promise.send_closure(td::actor::actor_id(this), &TonlibConsumer::got_prev_round2));
    }

    void got_prev_round2(tonlib_api::object_ptr<tonlib_api::tvm_cell> info, td::Promise<PrevRound> promise)
    {
      // SmartContract smc{state};
      // smc.run_getmethod('lucky_nums');
      // smc.run_getmethod('prizes');
      promise.set_error(td::Status::Error(10, "sdafsadfwer"));
    }

    void is_winner(std::string addr, td::Promise<std::int64_t> promise)
    {
      using tonlib_api::make_object;

      std::vector<tonlib_api::object_ptr<tonlib_api::tvm_StackEntry>> stack;

      // TODO: parse_stack_entry(addr)
      auto ff0 = make_object<tonlib_api::tvm_stackEntryNumber>(make_object<tonlib_api::tvm_numberDecimal>("0"));
      stack.push_back(std::move(ff0));

      auto num = td::RefInt256{true};
      auto& x = num.unique_write();
      x.parse_hex(addr.data(), addr.size());
      auto rr = dec_string(num);
      auto ff = make_object<tonlib_api::tvm_stackEntryNumber>(make_object<tonlib_api::tvm_numberDecimal>(rr));
      stack.push_back(std::move(ff));

      run_method("is_winner", std::move(stack),
              promise.send_closure(td::actor::actor_id(this), &TonlibConsumer::got_is_winner));
    }

    void got_is_winner(tonlib_api::object_ptr<tonlib_api::smc_runResult> info, td::Promise<std::int64_t> promise)
    {
      // TODO: store_entry(info->stack_[0])
      auto& entry = static_cast<tonlib_api::tvm_stackEntryNumber&>(*info->stack_[0]);
      promise.set_value(std::stoll(entry.number_->number_));
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

  router->http_get("/current", [&](auto req, auto) {
    g_scheduler_ptr->run_in_context_external([&] {
      td::actor::send_closure(tonClient, &TonlibConsumer::current_round,
          [req](td::Result<TonlibConsumer::CurrentRound> res) {
              if (res.is_error()) {
                  init_resp(req->create_response(restinio::status_internal_server_error()))
                          .done();
                  return;
              }
              
              auto r = res.move_as_ok();
              auto b = fmt::format(R"({{"prize_fund": {0}, "participants_count": {1}}})", r.prize_fund, r.participants_count);

            init_resp(req->create_response())
              .append_header(restinio::http_field::content_type, "text/json; charset=utf-8")
              .set_body(b)
              .done();
          });
    });

    return restinio::request_accepted();
  });

  router->http_get("/prev", [&](auto req, auto) {
    g_scheduler_ptr->run_in_context_external([&] {
      td::actor::send_closure(tonClient, &TonlibConsumer::prev_round,
          [req](td::Result<TonlibConsumer::PrevRound> res) {
              if (res.is_error()) {
                  init_resp(req->create_response(restinio::status_internal_server_error()))
                          .done();
                  return;
              }

              auto r = res.move_as_ok();

              std::stringstream ss;
              ss << R"({"lucky_nums": [)";
              for (auto num : r.lucky_nums) {
                  ss << num << ',';
              }
              //ss.removelast();
              ss << R"(], "prizes": [)";
              for (auto prize : r.prizes) {
                  ss << prize << ',';
              }
              //ss.removelast();
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
      td::actor::send_closure(tonClient, &TonlibConsumer::is_winner, std::move(addr),
          [req](td::Result<std::int64_t> res) {
            if (res.is_error()) {
                init_resp(req->create_response(restinio::status_internal_server_error()))
                        .done();
                return;
            }

            auto b = fmt::format(R"({{"prize": {}}})", res.ok());

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

    td::actor::Scheduler scheduler{{1}};
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
