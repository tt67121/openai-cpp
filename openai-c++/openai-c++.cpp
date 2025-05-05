#include "pch.h"
#include "framework.h"
#include "openai-c++.h"
#include "openai_client.h"
#include <vector>

Copenaic::Copenaic(const std::string& apiKey, const std::string& baseUrl)
    : client_(std::make_unique<OpenAIClient>(apiKey, baseUrl)) {
}

Copenaic::~Copenaic() = default;

std::string Copenaic::ChatCompletion(
    const std::string& model,
    const std::vector<std::string>& messages,
    float temperature,
    float top_p,
    int n,
    bool stream,
    const std::string& stop,
    int max_tokens,
    float presence_penalty,
    float frequency_penalty) {

    std::vector<OpenAIClient::ChatMessage> chatMessages;
    // 将消息转换为适当的格式
    for (size_t i = 0; i < messages.size(); i++) {
        OpenAIClient::ChatMessage msg;
        msg.role = (i % 2 == 0) ? "user" : "assistant";
        msg.content = messages[i];
        chatMessages.push_back(msg);
    }

    auto completion = client_->CreateChatCompletion(
        model,
        chatMessages,
        temperature,
        top_p,
        n,
        stream,
        stop,
        max_tokens,
        presence_penalty,
        frequency_penalty);

    // 返回第一个回复的内容
    return completion.choices.empty() ? "" : completion.choices[0].content;
}

std::vector<std::vector<float>> Copenaic::CreateEmbedding(
    const std::string& model,
    const std::string& input) {
    
    auto response = client_->CreateEmbedding(model, input);
    return response.data;
}

std::vector<std::string> Copenaic::CreateImage(
    const std::string& prompt,
    int n,
    const std::string& size,
    const std::string& response_format) {

    auto response = client_->CreateImage(prompt, n, size, response_format);
    return response.data;
}
