// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 OPENAIC_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// OPENAIC_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#include <string>
#include <vector>
#include <memory>

#ifdef OPENAIC_EXPORTS
#define OPENAIC_API __declspec(dllexport)
#else
#define OPENAIC_API __declspec(dllimport)
#endif

// 此类是从 dll 导出的
class OPENAIC_API Copenaic {
public:
    Copenaic(const std::string& apiKey, const std::string& baseUrl = "https://api.openai.com/v1");
    ~Copenaic();

    std::string ChatCompletion(
        const std::string& model,
        const std::vector<std::string>& messages,
        float temperature = 1.0f,
        float top_p = 1.0f,
        int n = 1,
        bool stream = false,
        const std::string& stop = "",
        int max_tokens = 0,
        float presence_penalty = 0.0f,
        float frequency_penalty = 0.0f);

    std::vector<std::vector<float>> CreateEmbedding(
        const std::string& model,
        const std::string& input);

    std::vector<std::string> CreateImage(
        const std::string& prompt,
        int n = 1,
        const std::string& size = "1024x1024",
        const std::string& response_format = "url");

private:
    std::unique_ptr<class OpenAIClient> client_;
};

