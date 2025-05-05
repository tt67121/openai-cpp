#include "pch.h"
#include "openai_client.h"
#include <stdexcept>
#include <sstream>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

OpenAIClient::OpenAIClient(const std::string& apiKey, const std::string& baseUrl)
    : apiKey_(apiKey), baseUrl_(baseUrl), httpClient_(std::make_unique<HttpClient>()) {}

std::string OpenAIClient::BuildUrl(const std::string& endpoint) const {
    return baseUrl_ + endpoint;
}

std::map<std::string, std::string> OpenAIClient::BuildHeaders() const {
    return {
        {"Authorization", "Bearer " + apiKey_},
        {"Content-Type", "application/json"}
    };
}

OpenAIClient::ChatCompletion OpenAIClient::CreateChatCompletion(
    const std::string& model,
    const std::vector<ChatMessage>& messages,
    float temperature,
    float top_p,
    int n,
    bool stream,
    const std::string& stop,
    int max_tokens,
    float presence_penalty,
    float frequency_penalty) {
    
    json requestBody = {
        {"model", model},
        {"messages", json::array()},
        {"temperature", temperature},
        {"top_p", top_p},
        {"n", n},
        {"stream", stream}
    };

    for (const auto& msg : messages) {
        requestBody["messages"].push_back({
            {"role", msg.role},
            {"content", msg.content}
        });
    }

    if (!stop.empty()) {
        requestBody["stop"] = stop;
    }
    if (max_tokens > 0) {
        requestBody["max_tokens"] = max_tokens;
    }
    if (presence_penalty != 0.0f) {
        requestBody["presence_penalty"] = presence_penalty;
    }
    if (frequency_penalty != 0.0f) {
        requestBody["frequency_penalty"] = frequency_penalty;
    }

    auto response = httpClient_->Post(
        BuildUrl("/chat/completions"),
        requestBody.dump(),
        BuildHeaders());

    if (response.status_code != 200) {
        std::cerr << "API request failed: " << response.body << std::endl;
        // throw std::runtime_error("API request failed: " + response.body);
    }

    json responseJson = json::parse(response.body);
    ChatCompletion completion;
    completion.id = responseJson["id"];
    completion.object = responseJson["object"];
    completion.created = responseJson["created"];
    completion.model = responseJson["model"];

    for (const auto& choice : responseJson["choices"]) {
        ChatMessage message;
        message.role = choice["message"]["role"];
        message.content = choice["message"]["content"];
        completion.choices.push_back(message);
    }

    auto usage = responseJson["usage"];
    completion.usage = {
        {"prompt_tokens", usage["prompt_tokens"]},
        {"completion_tokens", usage["completion_tokens"]},
        {"total_tokens", usage["total_tokens"]}
    };

    return completion;
}

OpenAIClient::EmbeddingResponse OpenAIClient::CreateEmbedding(
    const std::string& model,
    const std::string& input) {
    
    json requestBody = {
        {"model", model},
        {"input", input}
    };

    auto response = httpClient_->Post(
        BuildUrl("/embeddings"),
        requestBody.dump(),
        BuildHeaders());

    if (response.status_code != 200) {
        std::cerr << "API request failed: " << response.body << std::endl;
        throw std::runtime_error("API request failed: " + response.body);
    }

    json responseJson = json::parse(response.body);
    EmbeddingResponse embedding;
    embedding.object = responseJson["object"];
    embedding.model = responseJson["model"];

    for (const auto& data : responseJson["data"]) {
        embedding.data.push_back(data["embedding"].get<std::vector<float>>());
    }

    auto usage = responseJson["usage"];
    embedding.usage = {
        {"prompt_tokens", usage["prompt_tokens"]},
        {"total_tokens", usage["total_tokens"]}
    };

    return embedding;
}

OpenAIClient::ImageResponse OpenAIClient::CreateImage(
    const std::string& prompt,
    int n,
    const std::string& size,
    const std::string& response_format) {
    
    json requestBody = {
        {"prompt", prompt},
        {"n", n},
        {"size", size},
        {"response_format", response_format}
    };

    auto response = httpClient_->Post(
        BuildUrl("/images/generations"),
        requestBody.dump(),
        BuildHeaders());

    if (response.status_code != 200) {
        std::cerr << "API request failed: " << response.body << std::endl;
        throw std::runtime_error("API request failed: " + response.body);
    }

    json responseJson = json::parse(response.body);
    ImageResponse image;
    image.created = responseJson["created"];

    for (const auto& data : responseJson["data"]) {
        if (response_format == "url") {
            image.data.push_back(data["url"]);
        } else {
            image.data.push_back(data["b64_json"]);
        }
    }

    return image;
}