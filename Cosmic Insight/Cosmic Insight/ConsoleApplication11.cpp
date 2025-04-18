#include <iostream>
#include <string>
#include <curl/curl.h>
#include <map>
#include <windows.h>
#include <regex>
#include <sstream>
#include <chrono>
#include <ctime>
#include "json.hpp"

using json = nlohmann::json;

// Callback-функция для обработки ответа от API
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* data) {
    size_t totalSize = size * nmemb;
    data->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// Безопасное получение текущей даты
std::string getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    std::tm tmStruct = {};
    localtime_s(&tmStruct, &currentTime);

    char buffer[11];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &tmStruct);
    return std::string(buffer);
}

// Функция для отправки HTTP-запроса
std::string fetchHoroscope(const std::string& sign, const std::string& day) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        std::string url = "https://horoscope-app-api.vercel.app/api/v1/get-horoscope/daily?sign=" + sign + "&day=" + day;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "HoroscopeApp/1.0");

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            response = "Error: " + std::string(curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }
    else {
        response = "Failed to initialize CURL";
    }

    return response;
}

// Функция перевода текста
std::string translateText(const std::string& text) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        std::string encodedText = curl_easy_escape(curl, text.c_str(), text.length());
        std::string url = "https://api.mymemory.translated.net/get?q=" + encodedText + "&langpair=en|ru";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            response = "Translation error: " + std::string(curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }

    return response;
}

// Проверка формата даты
bool isValidDateFormat(const std::wstring& date) {
    std::wregex dateRegex(L"^\\d{4}-(0[1-9]|1[0-2])-(0[1-9]|[12][0-9]|3[01])$");
    return std::regex_match(date, dateRegex);
}

// Вывод цветного текста в консоль
void printWString(const std::wstring& str, WORD color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
    WriteConsoleW(hConsole, str.c_str(), static_cast<DWORD>(str.length()), nullptr, nullptr);
    WriteConsoleW(hConsole, L"\n", 1, nullptr, nullptr);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

// Чтение ввода пользователя
std::wstring readWString() {
    HANDLE hConsole = GetStdHandle(STD_INPUT_HANDLE);
    wchar_t buffer[256];
    DWORD charsRead;
    ReadConsoleW(hConsole, buffer, 256, &charsRead, nullptr);

    // Удаление символов новой строки
    if (charsRead > 0 && buffer[charsRead - 1] == L'\n') charsRead--;
    if (charsRead > 0 && buffer[charsRead - 1] == L'\r') charsRead--;

    return std::wstring(buffer, charsRead);
}

// Отображение меню знаков зодиака
void displayZodiacMenu(bool isRussian) {
    const wchar_t* signs[] = {
        L"1. Овен / Aries", L"2. Телец / Taurus", L"3. Близнецы / Gemini",
        L"4. Рак / Cancer", L"5. Лев / Leo", L"6. Дева / Virgo",
        L"7. Весы / Libra", L"8. Скорпион / Scorpio", L"9. Стрелец / Sagittarius",
        L"10. Козерог / Capricorn", L"11. Водолей / Aquarius", L"12. Рыбы / Pisces"
    };

    printWString(isRussian ? L"Выберите знак зодиака:" : L"Choose your zodiac sign:");
    for (const auto& sign : signs) {
        printWString(sign);
    }
}

// Маппинг знаков зодиака
const std::map<int, std::pair<std::string, std::wstring>> zodiacSigns = {
    {1, {"Aries", L"Овен"}}, {2, {"Taurus", L"Телец"}}, {3, {"Gemini", L"Близнецы"}},
    {4, {"Cancer", L"Рак"}}, {5, {"Leo", L"Лев"}}, {6, {"Virgo", L"Дева"}},
    {7, {"Libra", L"Весы"}}, {8, {"Scorpio", L"Скорпион"}}, {9, {"Sagittarius", L"Стрелец"}},
    {10, {"Capricorn", L"Козерог"}}, {11, {"Aquarius", L"Водолей"}}, {12, {"Pisces", L"Рыбы"}}
};

// Конвертация wstring в string
std::string wstringToString(const std::wstring& wstr) {
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string str(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size, nullptr, nullptr);
    return str.substr(0, size - 1);
}

// Конвертация string в wstring
std::wstring stringToWstring(const std::string& str) {
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size);
    return wstr.substr(0, size - 1);
}

// Извлечение данных гороскопа из JSON
bool parseHoroscope(const std::string& jsonStr, std::string& horoscope, std::string& date) {
    try {
        json j = json::parse(jsonStr);
        if (j.contains("data")) {
            horoscope = j["data"]["horoscope_data"].get<std::string>();
            date = j["data"]["date"].get<std::string>();
            return true;
        }
    }
    catch (...) {
        horoscope = "Error parsing horoscope data";
        date = "Unknown date";
    }
    return false;
}

// Основная функция
int main() {
    // Инициализация
    curl_global_init(CURL_GLOBAL_DEFAULT);
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // Выбор языка
    printWString(L"Выберите язык / Choose language (ru/en): ");
    std::wstring lang = readWString();
    bool isRussian = (lang == L"ru" || lang == L"RU" || lang == L"Ru");

    // Работа с датой
    std::string currentDate = getCurrentDate();
    printWString(isRussian ?
        L"Сегодняшняя дата: " + stringToWstring(currentDate) + L". Использовать её? (да/нет): " :
        L"Today's date: " + stringToWstring(currentDate) + L". Use it? (yes/no): ");

    std::wstring useToday = readWString();
    bool useCurrentDate = (useToday == L"да" || useToday == L"yes" || useToday == L"Да" || useToday == L"Yes");

    std::wstring dayInput;
    if (!useCurrentDate) {
        do {
            printWString(isRussian ?
                L"Введите дату (ГГГГ-ММ-ДД): " :
                L"Enter date (YYYY-MM-DD): ");
            dayInput = readWString();
        } while (!isValidDateFormat(dayInput));
    }
    else {
        dayInput = stringToWstring(currentDate);
    }

    // Выбор знака зодиака
    displayZodiacMenu(isRussian);
    printWString(isRussian ? L"Введите номер знака: " : L"Enter sign number: ");

    int zodiacNumber;
    try {
        zodiacNumber = std::stoi(readWString());
        if (zodiacSigns.find(zodiacNumber) == zodiacSigns.end()) {
            throw std::out_of_range("Invalid zodiac number");
        }
    }
    catch (...) {
        printWString(isRussian ? L"Неверный номер знака!" : L"Invalid sign number!", FOREGROUND_RED);
        curl_global_cleanup();
        return 1;
    }

    // Получение гороскопа
    std::string sign = zodiacSigns.at(zodiacNumber).first;
    std::string response = fetchHoroscope(sign, wstringToString(dayInput));

    if (response.empty() || response.find("Error") != std::string::npos) {
        printWString(stringToWstring(response), FOREGROUND_RED);
        curl_global_cleanup();
        return 1;
    }

    // Парсинг ответа
    std::string horoscopeText, horoscopeDate;
    if (!parseHoroscope(response, horoscopeText, horoscopeDate)) {
        printWString(isRussian ? L"Ошибка обработки гороскопа" : L"Error processing horoscope", FOREGROUND_RED);
        curl_global_cleanup();
        return 1;
    }

    // Вывод результатов
    if (isRussian) {
        // Перевод на русский
        std::string translation = translateText(horoscopeText);
        try {
            json j = json::parse(translation);
            if (j.contains("responseData") && j["responseData"].contains("translatedText")) {
                std::string translatedText = j["responseData"]["translatedText"];
                printWString(L"\n" + zodiacSigns.at(zodiacNumber).second + L" на " + stringToWstring(horoscopeDate) + L":");
                printWString(stringToWstring(translatedText), FOREGROUND_GREEN);
            }
            else {
                throw std::runtime_error("Invalid translation response");
            }
        }
        catch (...) {
            printWString(L"Ошибка перевода. Оригинальный текст:", FOREGROUND_RED);
            printWString(stringToWstring(horoscopeText));
        }
    }
    else {
        printWString(L"\n" + stringToWstring(sign) + L" on " + stringToWstring(horoscopeDate) + L":");
        printWString(stringToWstring(horoscopeText));
    }

    // Завершение
    curl_global_cleanup();
    return 0;
}