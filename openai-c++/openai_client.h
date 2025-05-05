#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include "http_client.h"

#ifdef OPENAIC_EXPORTS
#define OPENAIC_API __declspec(dllexport)
#else
#define OPENAIC_API __declspec(dllimport)
#endif

class OPENAIC_API OpenAIClient {
public:
    struct ChatMessage {
        std::string role;
        std::string content;
    };

    struct ChatCompletion {
        std::string id;
        std::string object;
        int64_t created;
        std::string model;
        std::vector<ChatMessage> choices;
        std::map<std::string, int> usage;
    };

    struct EmbeddingResponse {
        std::string object;
        std::vector<std::vector<float>> data;
        std::string model;
        std::map<std::string, int> usage;
    };

    struct ImageResponse {
        int64_t created;
        std::vector<std::string> data;  // URLs or base64 encoded images
    };

    OpenAIClient(const std::string& apiKey, const std::string& baseUrl = "https://api.openai.com/v1");

    // Chat Completions API
    ChatCompletion CreateChatCompletion(
        const std::string& model,
        const std::vector<ChatMessage>& messages,
        float temperature = 1.0f,
        float top_p = 1.0f,
        int n = 1,
        bool stream = false,
        const std::string& stop = "",
        int max_tokens = 0,
        float presence_penalty = 0.0f,
        float frequency_penalty = 0.0f);

    // Embeddings API
    EmbeddingResponse CreateEmbedding(
        const std::string& model,
        const std::string& input);

    // Images API
    ImageResponse CreateImage(
        const std::string& prompt,
        int n = 1,
        const std::string& size = "1024x1024",
        const std::string& response_format = "url");

private:
    std::string apiKey_;
    std::string baseUrl_;
    std::unique_ptr<HttpClient> httpClient_;

    std::string BuildUrl(const std::string& endpoint) const;
    std::map<std::string, std::string> BuildHeaders() const;
};