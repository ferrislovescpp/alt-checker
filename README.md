# minecraft alt checker

## features:
  * premium status check: this program uses the mojang api to check the premium status of a given username. it makes an http request to the api and examines the response code to determine if the account is premium or cracked

  * error handling: this program also handles various error scenarios gracefully. it checks if the necessary files are present and readable, and provides appropriate error messages if any issues occur during file access or json parsing.

  * caching: the program maintains a set of printed usernames to avoid duplicate entries. it compares the usernames extracted from the json data with the ones already printed to ensure each username is only printed once.

## requirements:

  * ***CURL***: a library for making http requests, i use it to interact with the mojang api to retrieve account information.

  * ***nlohmann/json***: a json library for C++, it provides convenient methods for parsing and manipulating json data.

## installation: 

you can either just install the alt-checker.zip file and run the .exe, or you can compile it yourself. 

1. compile the program using a C++ compiler, ensuring that the necessary dependencies are properly linked and included in the project settings.

## limitations:

  * this program assumes that the necessary json files (accounts.json, usercache.json, and usernamecache.json) follow a specific structure. any deviations from the expected structure may result in incorrect or incomplete account information.

  * this program relies on the availability and response of the mojang api to determine the premium status of the usernames. if the api is down or inaccessible, this program will sadly not work.

  * this program does not modify the json files, its primary purpose is to retrieve and display account information.
