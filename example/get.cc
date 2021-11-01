#include "WFWebServer.h"
#include "WFHttpMsg.h"
#include "workflow/WFFacilities.h"
#include "workflow/HttpUtil.h"
#include <csignal>
#include "json.hpp"

using namespace wfrest;

using json = nlohmann::json;

static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo)
{
    wait_group.done();
}

int main()
{
    signal(SIGINT, sig_handler);

    WFWebServer svr;

    // curl -v http://ip:port/hello
    svr.Get("/hello", [](const HttpReq *req, HttpResp *resp)
    {
        resp->String("world\n");
    });
    // curl -v http://ip:port/data
    svr.Get("/data", [](const HttpReq *req, HttpResp *resp)
    {
        resp->Data("Hello world\n", 12 /* true */);
    });

    svr.Get("/api/<name>", [](HttpReq *req, HttpResp *resp)
    {
        std::string name = req->query_params["name"];
        resp->String(name+"\n");
    });
    // We do not provide a built-in json library,
    // users can choose the json library according to their preferences
    // curl -v http://ip:port/json
    svr.Get("/json", [](const HttpReq *req, HttpResp *resp)
    {
        json js;
        js["test"] = 123;
        js["json"] = "test json";
        resp->String(js.dump());
    });

    // curl -v http://ip:port/html/index.html
    svr.Get("/html/index.html", [](const HttpReq *req, HttpResp *resp)
    {
        resp->File("html/index.html");
    });

    // curl -v http://ip:port/post -d 'post hello world'
    svr.Post("/post", [](const HttpReq *req, HttpResp *resp)
    {
        const char *body;
        size_t body_len = 0;
        req->Body(&body, &body_len);
        fprintf(stderr, "post data : %s\n", body);
    });

    // Content-Type: application/x-www-form-urlencoded
    // curl -v http://ip:port/enlen -H "content-type:application/x-www-form-urlencoded" -d 'user=admin&pswd=123456'
    svr.Post("/enlen", [](const HttpReq *req, HttpResp *resp)
    {
        protocol::HttpHeaderCursor cursor(req);
        std::string content_type;
        cursor.find("Content-Type", content_type);
        if (content_type != "application/x-www-form-urlencoded")
        {
            resp->set_status(HttpStatusBadRequest);
            return;
        }
        const char *body;
        size_t body_len = 0;
        req->Body(&body, &body_len);
        fprintf(stderr, "post data : %s\n", body);
    });

    if (svr.start(9001) == 0)
    {
        wait_group.wait();
        svr.stop();
    } else
    {
        fprintf(stderr, "Cannot start server");
        exit(1);
    }

}