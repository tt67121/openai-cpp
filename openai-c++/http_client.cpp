#include "pch.h"
#include "http_client.h"
#include <stdexcept>
#include <vector>
#include <sstream>

HttpClient::HttpClient() {
    hSession_ = WinHttpOpen(
        L"OpenAI C++ Client/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);

    if (!hSession_) {
        throw std::runtime_error("Failed to initialize WinHTTP session");
    }
}

HttpClient::~HttpClient() {
    if (hSession_) {
        WinHttpCloseHandle(hSession_);
    }
}

HttpClient::Response HttpClient::Get(const std::string& url, const std::map<std::string, std::string>& headers) {
    return SendRequest("GET", url, "", headers);
}

HttpClient::Response HttpClient::Post(const std::string& url, const std::string& body, const std::map<std::string, std::string>& headers) {
    return SendRequest("POST", url, body, headers);
}

HttpClient::Response HttpClient::Delete(const std::string& url, const std::map<std::string, std::string>& headers) {
    return SendRequest("DELETE", url, "", headers);
}

HttpClient::Response HttpClient::SendRequest(const std::string& method, const std::string& url, const std::string& body, const std::map<std::string, std::string>& headers) {
    std::wstring host, path;
    INTERNET_PORT port;
    bool isHttps;
    ParseUrl(url, host, path, port, isHttps);

    HINTERNET hConnect = WinHttpConnect(hSession_, host.c_str(), port, 0);
    if (!hConnect) {
        throw std::runtime_error("Failed to connect to server");
    }

    DWORD flags = WINHTTP_FLAG_REFRESH;
    if (isHttps) flags |= WINHTTP_FLAG_SECURE;

    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect,
        std::wstring(method.begin(), method.end()).c_str(),
        path.c_str(),
        NULL,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        flags);

    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        throw std::runtime_error("Failed to create request");
    }

    // Add headers
    for (const auto& header : headers) {
        std::wstring headerStr = std::wstring(header.first.begin(), header.first.end()) +
            L": " + std::wstring(header.second.begin(), header.second.end());
        WinHttpAddRequestHeaders(hRequest, headerStr.c_str(), -1L, WINHTTP_ADDREQ_FLAG_ADD);
    }

    // Send request
    BOOL result;
    if (!body.empty()) {
        result = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            (LPVOID)body.c_str(), body.length(), body.length(), 0);
    } else {
        result = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    }

    if (!result) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        throw std::runtime_error("Failed to send request");
    }

    result = WinHttpReceiveResponse(hRequest, NULL);
    if (!result) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        throw std::runtime_error("Failed to receive response");
    }

    // Get status code
    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(DWORD);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX);

    // Read response body
    std::string responseBody;
    DWORD bytesAvailable;
    do {
        bytesAvailable = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &bytesAvailable)) {
            break;
        }

        if (!bytesAvailable) {
            break;
        }

        std::vector<char> buffer(bytesAvailable);
        DWORD bytesRead = 0;
        if (WinHttpReadData(hRequest, buffer.data(), bytesAvailable, &bytesRead)) {
            responseBody.append(buffer.data(), bytesRead);
        }
    } while (bytesAvailable > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);

    Response response;
    response.status_code = statusCode;
    response.body = responseBody;
    return response;
}

void HttpClient::ParseUrl(const std::string& url, std::wstring& host, std::wstring& path, INTERNET_PORT& port, bool& isHttps) {
    URL_COMPONENTS urlComp = { 0 };
    urlComp.dwStructSize = sizeof(urlComp);

    wchar_t hostName[256] = { 0 };
    wchar_t urlPath[1024] = { 0 };
    wchar_t scheme[32] = { 0 };

    urlComp.lpszHostName = hostName;
    urlComp.dwHostNameLength = sizeof(hostName) / sizeof(hostName[0]);
    urlComp.lpszUrlPath = urlPath;
    urlComp.dwUrlPathLength = sizeof(urlPath) / sizeof(urlPath[0]);
    urlComp.lpszScheme = scheme;
    urlComp.dwSchemeLength = sizeof(scheme) / sizeof(scheme[0]);

    std::wstring wUrl(url.begin(), url.end());
    if (!WinHttpCrackUrl(wUrl.c_str(), 0, 0, &urlComp)) {
        throw std::runtime_error("Failed to parse URL");
    }

    host = hostName;
    path = urlPath;
    port = urlComp.nPort;
    isHttps = (wcscmp(scheme, L"https") == 0);
}