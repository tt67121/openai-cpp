#pragma once

#include <string>
#include <map>
#include <memory>
#include <functional>
#include <winhttp.h>

#ifdef OPENAIC_EXPORTS
#define OPENAIC_API __declspec(dllexport)
#else
#define OPENAIC_API __declspec(dllimport)
#endif

class OPENAIC_API HttpClient {
public:
    struct Response {
        int status_code;
        std::string body;
        std::map<std::string, std::string> headers;
    };

    HttpClient();
    ~HttpClient();

    Response Get(const std::string& url, const std::map<std::string, std::string>& headers = {});
    Response Post(const std::string& url, const std::string& body, const std::map<std::string, std::string>& headers = {});
    Response Delete(const std::string& url, const std::map<std::string, std::string>& headers = {});

private:
    Response SendRequest(const std::string& method, const std::string& url, const std::string& body, const std::map<std::string, std::string>& headers);
    void ParseUrl(const std::string& url, std::wstring& host, std::wstring& path, INTERNET_PORT& port, bool& isHttps);

    HINTERNET hSession_;
};