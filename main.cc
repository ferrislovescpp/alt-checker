
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <iostream>
#include <cstdlib>  
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::string ACCOUNTS_FILE_PATH;
std::string USERCACHE_FILE_PATH;
std::string USERNAMECACHE_FILE_PATH;

size_t write_data(void* ptr, size_t size, size_t nmemb, std::stringstream* stream) {
    size_t written = size * nmemb;
    stream->write(static_cast<const char*>(ptr), written);
    return written;
}

std::string isPremium(const std::string& username) {
    std::string url = "https://api.mojang.com/users/profiles/minecraft/" + username;
    CURL* curl = curl_easy_init();
    if (curl) {
        std::stringstream out;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
        CURLcode res = curl_easy_perform(curl);
        long status_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
        curl_easy_cleanup(curl);
        if (res == CURLE_OK && status_code == 200) {
            return "premium";
        }
        else {
            return "cracked";
        }
    }
    return "unknown";
}

std::set<std::string> printAccounts(json& accounts) {
    std::set<std::string> printed_usernames;
    auto accounts_iter = accounts.find("accounts");
    if (accounts_iter != accounts.end()) {
        auto accounts_dict = *accounts_iter;
        for (auto& account : accounts_dict.items()) {
            auto account_dict = account.value();
            auto username_iter = account_dict.find("username");
            if (username_iter != account_dict.end()) {
                std::string username = *username_iter;
                std::string is_prem = !account_dict.count("accessToken") ? "premium" : "cracked";
                std::cout << username << " (" << is_prem << ")" << std::endl;
                printed_usernames.insert(username);
            }
        }
    }
    else {
        std::cout << "no lunar client accounts found in the file." << std::endl;
    }
    return printed_usernames;
}

std::set<std::string> printCache(const std::string& json_file_path, const std::set<std::string>& printed_usernames, const std::string& key = "name") {
    std::set<std::string> updated_set = printed_usernames;
    std::ifstream file(json_file_path);
    if (file.good()) {
        json data;
        try {
            file >> data;
            for (auto& item : data) {
                std::string username = item.value(key, "");
                if (username != "" && updated_set.find(username) == updated_set.end()) {
                    std::string is_prem = isPremium(username);
                    std::cout << username << " (" << is_prem << ")" << std::endl;
                    updated_set.insert(username);
                }
            }
        }
        catch (json::parse_error& e) {
            std::cout << "failed to decode JSON data in " << json_file_path << "." << std::endl;
        }
    }
    else {
        std::cout << json_file_path << " not found." << std::endl;
    }
    return updated_set;
}

int main() {
    char userProfile[MAX_PATH];
    size_t len;
    char* userProfilePtr = nullptr;
    _dupenv_s(&userProfilePtr, &len, "USERPROFILE");

    if (userProfilePtr != nullptr) { // check if userProfilePtr is not nullptr
        strcpy_s(userProfile, MAX_PATH, userProfilePtr);
        free(userProfilePtr);

        ACCOUNTS_FILE_PATH = std::string(userProfile) + "\\settings\\game\\accounts.json";
        USERCACHE_FILE_PATH = std::string(std::getenv("APPDATA")) + "\\.minecraft\\usercache.json";
        USERNAMECACHE_FILE_PATH = std::string(std::getenv("APPDATA")) + "\\.minecraft\\usernamecache.json";

        json accounts;
        std::ifstream accounts_file(ACCOUNTS_FILE_PATH);

        if (accounts_file.good()) {
            try {
                accounts_file >> accounts;
                std::set<std::string> printed_usernames = printAccounts(accounts);
                auto userAccount_iter = accounts.find("userAccount");
                if (userAccount_iter != accounts.end()) {
                    auto userAccount_dict = *userAccount_iter;
                    auto username_iter = userAccount_dict.find("username");
                    if (username_iter != userAccount_dict.end()) {
                        std::string username = *username_iter;
                        std::cout << "added lunar client username: " << username << std::endl;
                        printed_usernames.insert(username);
                    }
                }
            }
            catch (json::parse_error& e) {
                std::cout << "failed to decode JSON data in " << ACCOUNTS_FILE_PATH << "." << std::endl;
                return EXIT_FAILURE;
            }
        }
        else {
            std::cout << "lunar client file not found." << std::endl;
        }

        std::set<std::string> updated_set = printCache(USERCACHE_FILE_PATH, std::set<std::string>());
        printCache(USERNAMECACHE_FILE_PATH, updated_set);

        system("pause");
        return EXIT_SUCCESS;
    }
    else {
        std::cout << "USERPROFILE environment variable not found." << std::endl;
        return EXIT_FAILURE;
    }
}