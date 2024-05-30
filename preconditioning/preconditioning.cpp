#include <iostream>
#include <filesystem>
#include <fstream>
#include <regex>
#include "chatBot.h"
#include "document.h"

namespace fs = std::filesystem;

const int COMMENT_THRESHOLD = 10;
const double RELEVANCY_THRESHOLD = 0.6;

std::string readUtf8File(const char* filename)
{
    std::ifstream ifs(filename);
    std::string content((std::istreambuf_iterator<char>(ifs)),
        (std::istreambuf_iterator<char>()));
    int widesize = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, content.c_str(), -1, NULL, 0);
    if (widesize == 0)
    {
        std::cerr << "Invalid UTF-8 string." << std::endl;
        return "";
    }
    std::vector<wchar_t> widestrbuf(widesize);
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, content.c_str(), -1, &widestrbuf[0], widesize);
    int narrowsize = WideCharToMultiByte(CP_ACP, 0, &widestrbuf[0], -1, NULL, 0, NULL, NULL);
    if (narrowsize == 0)
    {
        std::cerr << "Failed to convert wstring to local encoding." << std::endl;
        return "";
    }
    std::vector<char> narrowstrbuf(narrowsize);
    WideCharToMultiByte(CP_ACP, 0, &widestrbuf[0], -1, &narrowstrbuf[0], narrowsize, NULL, NULL);
    return std::string(narrowstrbuf.begin(), narrowstrbuf.end() - 1);
}

double extractRelevancyScore(const std::string& response) {
    std::regex pattern(R"(relevancy:(\d\.\d{2});)");
    std::smatch match;
    if (std::regex_search(response, match, pattern)) {
        std::string scoreStr = match[1].str();
        return std::stod(scoreStr);
    }
    return 0.0;
}

int main() {
    ChatBot chatBot;
    std::string systemMessage = "你是一个自然语言处理器,你会接收一个JSON格式的Bilibili视频数据,你需要用0-1的小数输出一个数字表示此视频与\"心理\"话题的相关度。输出示例:'relevancy:0.82;','relevancy:0.34;','relevancy:0.55;'。请注意,你只能回复'relevancy:0.**'";
    chatBot.initBot(systemMessage);

    std::string dataPath = "./BilibiliData";

    for (const auto& entry : fs::directory_iterator(dataPath)) {
        if (entry.is_directory()) {
            std::string videoInfoPath = entry.path().string() + "/" + entry.path().filename().string() + "_video_info.json";
            if (fs::exists(videoInfoPath)) {
                std::string videoInfoJson = readUtf8File(videoInfoPath.c_str());

                rapidjson::Document videoInfo;
                videoInfo.Parse(videoInfoJson.c_str());

                int commentCount = videoInfo["reply"].GetInt();
                if (commentCount < COMMENT_THRESHOLD) {
                    std::cout << "Video: " << entry.path().filename().string() << " - Comments: " << commentCount << " - Action: Delete" << std::endl;
                    fs::remove_all(entry.path());
                    continue;
                }

                std::string response = chatBot.chatWithUserInput(videoInfoJson);
                double relevancyScore = extractRelevancyScore(response);

                std::cout << "Video: " << entry.path().filename().string() << " title: " << videoInfo["title"].GetString() << " - Relevancy Score: " << relevancyScore;
                if (relevancyScore < RELEVANCY_THRESHOLD) {
                    std::cout << " - Action: Delete" << std::endl;
                    fs::remove_all(entry.path());
                }
                else {
                    std::cout << " - Action: Keep" << std::endl;
                }
            }
        }
    }

    return 0;
}