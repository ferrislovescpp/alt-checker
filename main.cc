
// disables annoying compile time errors/warnings
#define _CRT_SECURE_NO_WARNINGS 

// basic cpp standard includes
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>

#include <curl/curl.h>              // http requests library
#include <nlohmann/json.hpp>        // json library

using json = nlohmann::json;        // alias for nlohmann::json

// this is a function to write received data to a stringstream
size_t writeData(void* ptr, size_t size, size_t nmemb, std::stringstream* stream) {
    size_t written = size * nmemb;
    stream->write(static_cast<const char*>(ptr), written);
    return written;
}

// this is a function to check if the username is a premium or cracked acc
std::string isPremium(const std::string& username) {
    std::string url = "https://api.mojang.com/users/profiles/minecraft/" + username;

    // initialize a CURL handle for making HTTP requests
    CURL* curl = curl_easy_init();

    if (curl) {
        std::stringstream out;

        // set the URL to fetch
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // set the option to follow any HTTP redirects
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // set the callback function to write received data to the stringstream
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);

        // set the stringstream as the write data
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);

        // perform the request and capture the response
        CURLcode res = curl_easy_perform(curl);

        long status_code = 0;

        // get the response code
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);

        // cleanup the CURL handle
        curl_easy_cleanup(curl);

        // check if the request was successful and the response code is 200 
        if (res == CURLE_OK && status_code == 200) {
            return "premium";
        }
        else {
            return "cracked";
        }
    }

    // return "unknown" if CURL initialization failed
    return "unknown";
}

std::set<std::string> printAccounts(json& accounts) {
    // create a set to store the printed usernames
    std::set<std::string> printed_usernames;

    // find the "accounts" key in the provided JSON object
    auto accounts_iter = accounts.find("accounts");

    // check if the "accounts" key exists
    if (accounts_iter != accounts.end()) {
        // get the value associated with the "accounts" key
        auto accounts_dict = *accounts_iter;

        // iterate over each item in the "accounts" dictionary
        for (auto& account : accounts_dict.items()) {
            // get the value of the current account
            auto account_dict = account.value();

            // find the "username" key in the account dictionary
            auto username_iter = account_dict.find("username");

            // check if the "username" key exists
            if (username_iter != account_dict.end()) {
                // get the value associated with the "username" key
                std::string username = *username_iter;

                // determine the premium status based on the presence of the "accessToken" key
                std::string is_prem = !account_dict.count("accessToken") ? "premium" : "cracked";

                // print the username and its premium status
                std::cout << username << " (" << is_prem << ")" << std::endl;

                // insert the username into the set of printed usernames
                printed_usernames.insert(username);
            }
        }
    }
    else {
        // handle the case where no lunar client accounts are found in the file
        std::cout << "no lunar client accounts found in the file." << std::endl;
    }

    // return the set of printed usernames
    return printed_usernames;
}

std::set<std::string> printCache(const std::string& json_file_path, const std::set<std::string>& printed_usernames, const std::string& key = "name", std::nullptr_t = nullptr) {
    // create a set to store the updated usernames
    std::set<std::string> updated_set = printed_usernames;

    // create a unique_ptr to manage the ifstream object for the file stream
    std::unique_ptr<std::ifstream> fileStream(new std::ifstream(json_file_path));

    // get a reference to the file stream
    std::ifstream& file = *fileStream;

    // check if the file stream is in a good state
    if (file.good()) {
        // declare a JSON object to store the parsed data
        json data;
        try {
            // read the JSON data from the file into the JSON object
            file >> data;

            // iterate over each item in the JSON object
            for (auto& item : data) {
                // extract the username from the item using the specified key
                std::string username = item.value(key, "");

                // check if the username is not empty and is not already in the updated set
                if (username != "" && updated_set.find(username) == updated_set.end()) {
                    // get the premium status for the username
                    std::string is_prem = isPremium(username);

                    // print the username and its premium status
                    std::cout << username << " (" << is_prem << ")" << std::endl;

                    // add the username to the updated set
                    updated_set.insert(username);
                }
            }
        }
        catch (json::parse_error& e) {
            // handle the case where JSON parsing fails
            std::cout << "failed to decode JSON data in " << json_file_path << "." << std::endl;
        }
    }
    else {
        // handle the case where the file stream is not good (file not found or other error)
        std::cout << json_file_path << " not found." << std::endl;
    }

    // return the set of updated usernames
    return updated_set;
}


// main function
int main() {
    // get user's profile directory
    char userProfile[MAX_PATH];
    size_t len;
    char* userProfilePtr = nullptr;
    _dupenv_s(&userProfilePtr, &len, "USERPROFILE");

    // check if userProfilePtr is not nullptr
    if (userProfilePtr != nullptr) {
        // copy the user's profile directory to userProfile
        strcpy_s(userProfile, MAX_PATH, userProfilePtr);
        // free the memory allocated for the pointer to the user's profile directory
        free(userProfilePtr);

        // set the paths to the files that will be manipulated
        const std::string accountsFilePath = std::string(userProfile) + "\\settings\\game\\accounts.json";
        const std::string usercacheFilePath = std::string(std::getenv("APPDATA")) + "\\.minecraft\\usercache.json";
        const std::string usernamecacheFilePath = std::string(std::getenv("APPDATA")) + "\\.minecraft\\usernamecache.json";

        // parse the JSON data in the accounts file and print the account details
        json accounts;
        std::ifstream accountsFile(accountsFilePath);
        if (accountsFile.good()) {
            try {
                accountsFile >> accounts;
                // set to keep track of printed usernames
                std::set<std::string> printedUsernames = printAccounts(accounts);
                // get the userAccount dictionary and print the added Lunar Client username
                auto userAccountIter = accounts.find("userAccount");
                if (userAccountIter != accounts.end()) {
                    auto userAccountDict = *userAccountIter;
                    auto usernameIter = userAccountDict.find("username");
                    if (usernameIter != userAccountDict.end()) {
                        std::string username = *usernameIter;
                        std::cout << "added Lunar Client username: " << username << std::endl;
                        printedUsernames.insert(username);
                    }
                }
            }
            catch (json::parse_error& e) {
                std::cout << "failed to decode JSON data in " << accountsFilePath << "." << std::endl;
                return EXIT_FAILURE;
            }
            accountsFile.close();
        }
        else {
            std::cout << "Lunar Client file not found." << std::endl;
        }

        // update the cache and print the usernames
        std::set<std::string> updatedSet = printCache(usercacheFilePath, std::set<std::string>(), "name");
        printCache(usernamecacheFilePath, updatedSet, "name");

        // wait for user input before exiting
        std::cout << "press any key to exit..." << std::endl;
        std::cin.get();

        return EXIT_SUCCESS;
    }
    else {
        std::cout << "USERPROFILE environment variable not found." << std::endl;
        return EXIT_FAILURE;
    }
}
