#include "ServerForm.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <Windows.h>
#include <winhttp.h>
#include <string>
#include <iomanip> 
#include <sstream> 
#using <System.Net.Sockets.dll>
#using <System.Net.dll>
#using <System.Text.RegularExpressions.dll>
#using <System.IO.dll>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winhttp.lib")

using namespace System;
using namespace System::Net;
using namespace System::Net::Sockets;
using namespace System::Text;
using namespace System::Threading;
using namespace System::Windows::Forms;
using namespace System::Drawing;
using namespace msclr::interop;
using namespace System::Threading::Tasks;
#include "ServerForm.h"
#include <string>
#include <regex>
#include <fstream>
#include <msclr/marshal_cppstd.h>
#include <thread>
#include <list>
#include <vector>
#include <array>
#using <System.Net.Sockets.dll>
#using <System.Net.dll>
#using <System.Text.RegularExpressions.dll>
#using <System.IO.dll>

using namespace System;
using namespace System::Net;
using namespace System::Net::Sockets;
using namespace System::Text;
using namespace System::Threading;
using namespace System::Windows::Forms;
using namespace System::Drawing;
using namespace msclr::interop;
using namespace System::Threading::Tasks;


void Server::ServerForm::ServerForm_Load(System::Object^ sender, System::EventArgs^ e) {
    instance = this;
    Task::Run(gcnew Func<Task^>(this, &Server::ServerForm::Main));
}

System::Void Server::ServerForm::button1_Click(System::Object^ sender, System::EventArgs^ e) {
    if (moderate == false) {
        label5->Text = "ON";
        label5->ForeColor = System::Drawing::Color::Green;
        moderate = true;
    }
    else {
        label5->Text = "OFF";
        label5->ForeColor = System::Drawing::Color::Red;
        moderate = false;
    }
}

System::Void Server::ServerForm::button2_Click(System::Object^ sender, System::EventArgs^ e) {
    // TODO: Implement functionality
}

System::Void Server::ServerForm::button3_Click(System::Object^ sender, System::EventArgs^ e) {
    // TODO: Implement functionality
}

#include <msclr/marshal_cppstd.h>
#include <regex>
#include <string>

void Server::ServerForm::FindIP() {
    try {
        std::string command = "ipconfig /all";
        std::array<char, 128> buffer;
        std::string result;

        FILE* pipe = _popen(command.c_str(), "r");
        if (!pipe) throw std::runtime_error("popen() failed!");
        try {
            while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
                result += buffer.data();
            }
        }
        catch (...) {
            _pclose(pipe);
            throw;
        }
        _pclose(pipe);

        std::string output = result;
        std::regex rgx("Wireless LAN adapter Wi-Fi[\\s\\S]*?IPv4 Address[.\\s]+: ([0-9\\.]+)");
        std::smatch match;

        if (std::regex_search(output, match, rgx)) {
            // Convert the matched sub_match to a std::string
            std::string ip_str = match[1].str();

            // Convert the std::string to System::String^ using marshal_as
            String^ ip = marshal_as<String^>(ip_str);

            // Safely update the label from the UI thread
            this->Invoke(gcnew Action<String^>(this, &Server::ServerForm::UpdateLabel), ip);

            System::Net::IPAddress^ ipAddress = System::Net::IPAddress::Parse(ip);
        }
        else {
            // If no IP address is found, update the label with "Not Found"
            this->Invoke(gcnew Action<String^>(this, &Server::ServerForm::UpdateLabel), "Not Found");
        }
    }
    catch (Exception^ ex) {
        MessageBox::Show("Error: " + ex->Message, "FindIP Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
    }
}

void Server::ServerForm::UpdateLabel(String^ ip) {
    if (this->InvokeRequired) {
        this->Invoke(gcnew Action<String^>(this, &Server::ServerForm::UpdateLabel), ip);
    }
    else {
        label4->Text = ip;
    }
}



Task^ Server::ServerForm::Main() {
    FindIP();
    if (ipAddress == nullptr) {
        ipAddress = System::Net::IPAddress::Parse("192.168.203.231");
        this->Invoke(gcnew Action<String^>(this, &Server::ServerForm::UpdateLabel), "192.168.203.231");
    }

    try {
        TcpListener^ server = gcnew TcpListener(ipAddress, port);
        server->Server->SetSocketOption(SocketOptionLevel::Socket, SocketOptionName::ReuseAddress, true);
        server->Start();
        Console::WriteLine("Server is listening on {0}:{1}", ipAddress, port);

        // Приймаємо клієнтів у нескінченному циклі
        while (true) {
            TcpClient^ client = server->AcceptTcpClient(); // Очікуємо клієнта
            clients->Add(client); // Додаємо до списку клієнтів
            Console::WriteLine("Client connected.");

            // Запускаємо асинхронну обробку клієнта
            Task::Factory->StartNew(gcnew Action<Object^>(this, &Server::ServerForm::ProcessClient), client);
        }
    }
    catch (SocketException^ ex) {
        MessageBox::Show("Socket Exception: " + ex->Message, "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
    }
    catch (Exception^ ex) {
        MessageBox::Show("Exception: " + ex->Message, "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
    }

    return Task::CompletedTask;
}

String^ Server::ServerForm::CensorBadWords(String^ input, System::Collections::Generic::List<String^>^ badWords) {
    String^ censoredMessage = input;
    for each (String ^ word in badWords) {
        String^ pattern = "\\b" + word + "\\b";
        censoredMessage = System::Text::RegularExpressions::Regex::Replace(censoredMessage, pattern, "***", System::Text::RegularExpressions::RegexOptions::IgnoreCase);
        bdwordcount++;
    }
    return censoredMessage;
}

void Server::ServerForm::UpdateDataGridView() {
    try {
        // Ensure logined list has complete user data
        if (logined->Count % 3 != 0) {
            MessageBox::Show("Internal error: logined list has incomplete user data.", "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
            return;
        }

        int userCount = logined->Count / 3;

        // Ensure the DataGridView has at least two columns
        if (this->dataGridView1->Columns->Count < 2) {
            this->dataGridView1->Columns->Clear();
            this->dataGridView1->Columns->Add("UsernameColumn", "Username");
            this->dataGridView1->Columns->Add("DataColumn", "Additional Data");
        }

        // Set the RowCount
        this->dataGridView1->RowCount = userCount;

        for (int h = 0; h < userCount; h++) {
            // Access indices safely
            int usernameIndex = 3 * h;
            int additionalDataIndex = 3 * h + 2;

            // Ensure indices are within bounds
            if (usernameIndex >= logined->Count || additionalDataIndex >= logined->Count) {
                MessageBox::Show("Internal error: Index out of bounds when accessing logined list.", "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
                return;
            }

            String^ username = logined[usernameIndex];
            String^ additionalData = logined[additionalDataIndex];

            // Ensure Rows[h] exists
            if (h >= this->dataGridView1->Rows->Count) {
                MessageBox::Show("Internal error: DataGridView row index out of bounds.", "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
                return;
            }

            // Update the DataGridView cells
            this->dataGridView1->Rows[h]->Cells[0]->Value = username;
            this->dataGridView1->Rows[h]->Cells[1]->Value = additionalData;
        }
    }
    catch (Exception^ ex) {
        MessageBox::Show("Error updating DataGridView: " + ex->Message, "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
    }
}


// Method to prompt the user for signup confirmation
bool Server::ServerForm::PromptUserForSignupConfirmation(String^ username) {
    System::Windows::Forms::DialogResult res = System::Windows::Forms::MessageBox::Show(
        String::Format("Client: {0} wants to register (Confirm?)", username),
        "Confirmation",
        System::Windows::Forms::MessageBoxButtons::YesNo,
        System::Windows::Forms::MessageBoxIcon::Question
    );
    return (res == System::Windows::Forms::DialogResult::Yes);
}


void Server::ServerForm::ProcessClient(Object^ clientObj) {
    TcpClient^ client = dynamic_cast<TcpClient^>(clientObj);
    if (client == nullptr) return;

    try {
        NetworkStream^ stream = client->GetStream();
        array<Byte>^ buffer = gcnew array<Byte>(1024);

        int bytesRead;
        while ((bytesRead = stream->Read(buffer, 0, buffer->Length)) > 0) {
            // Convert bytes to string
            String^ data = Encoding::UTF8->GetString(buffer, 0, bytesRead);
            Console::WriteLine("Received: {0}", data);

            if (!String::IsNullOrEmpty(data)) {
                if (data[0] == 'L') {
                    // Login logic
                    int ch = 0;
                    String^ filePath = "users_data.txt";
                    array<String^>^ check = data->Split(' ');

                    String^ content = System::IO::File::ReadAllText(filePath);
                    array<String^>^ words = content->Split(gcnew array<wchar_t>{ ' ', '\r', '\n' }, StringSplitOptions::RemoveEmptyEntries);

                    for (int i = 0; i < words->Length; i += 2) {
                        if ((check[1] == words[i] && check[2] == words[i + 1])) {
                            int logcheck = 0;
                            for (int j = 0; j < logined->Count; j += 3) {
                                if ((check[1] == logined[j] && check[2] == logined[j + 1])) {
                                    logcheck++;
                                }
                            }
                            if (logcheck == 0) {
                                String^ saved = System::IO::File::ReadAllText("received_data.txt");
                                logined->Add(check[1]);
                                logined->Add(check[2]);
                                logined->Add(check[3]);

                                // Update DataGridView safely
                                this->Invoke(gcnew Action(this, &Server::ServerForm::UpdateDataGridView));

                                ch++;
                                String^ responseStr = "1 " + saved;
                                array<Byte>^ response = Encoding::UTF8->GetBytes(responseStr);
                                stream->Write(response, 0, response->Length);
                            }
                            else {
                                array<Byte>^ response = Encoding::UTF8->GetBytes("2 ");
                                stream->Write(response, 0, response->Length);
                            }
                        }
                    }

                    if (ch == 0) {
                        array<Byte>^ response = Encoding::UTF8->GetBytes("0 log in error");
                        stream->Write(response, 0, response->Length);
                    }
                }
                else if (data[0] == 'S') {
                    // Signup logic
                    int ch = 0;
                    String^ filePath = "users_data.txt";
                    array<String^>^ check = data->Split(' ');

                    String^ content = System::IO::File::ReadAllText(filePath);
                    array<String^>^ words = content->Split(gcnew array<wchar_t>{ ' ', '\r', '\n' }, StringSplitOptions::RemoveEmptyEntries);

                    for (int i = 0; i < words->Length; i += 2) {
                        if (check[1] == words[i]) {
                            ch++;
                            array<Byte>^ response = Encoding::UTF8->GetBytes("2 sign in error");
                            stream->Write(response, 0, response->Length);
                            break;
                        }
                    }

                    if (ch == 0) {
                        // Prompt for confirmation on the UI thread
                        Object^ objResult = this->Invoke(
                            gcnew PromptUserDelegate(this, &Server::ServerForm::PromptUserForSignupConfirmation),
                            gcnew array<Object^>{ check[1] }
                        );
                        bool result = safe_cast<bool>(objResult);

                        if (result) {
                            String^ modifiedData = data->Substring(2);
                            System::IO::File::AppendAllText("users_data.txt", "\n" + modifiedData);
                            String^ saved = System::IO::File::ReadAllText("received_data.txt");

                            logined->Add(check[1]);
                            logined->Add(check[2]);
                            logined->Add(check[3]);

                            // Update DataGridView safely
                            this->Invoke(gcnew Action(this, &Server::ServerForm::UpdateDataGridView));

                            String^ responseStr = "1 " + saved;
                            array<Byte>^ response = Encoding::UTF8->GetBytes(responseStr);
                            stream->Write(response, 0, response->Length);
                        }
                        else {
                            array<Byte>^ response = Encoding::UTF8->GetBytes("6 ");
                            stream->Write(response, 0, response->Length);
                        }
                    }
                }
                else if (data[0] == 'T') {
                    // Message transfer logic
                    String^ filePath = "received_data.txt";

                    String^ modifiedData = data->Substring(2);

                    if (moderate) {
                        // Before calling CensorBadWords
                        System::Collections::Generic::List<String^>^ badWords = gcnew System::Collections::Generic::List<String^>();
                        badWords->Add("fuck");
                        badWords->Add("shit");
                        badWords->Add("ass");

                        modifiedData = CensorBadWords(modifiedData, badWords);


                        if (bdwordcount < 3) {
                            System::IO::File::AppendAllText(filePath, "\n" + modifiedData);
                            String^ saved = System::IO::File::ReadAllText("received_data.txt");

                            String^ responseStr = "1S " + saved;
                            array<Byte>^ response = Encoding::UTF8->GetBytes(responseStr);

                            // Send to all connected clients
                            for each (TcpClient ^ connectedClient in clients) {
                                NetworkStream^ connectedStream = connectedClient->GetStream();
                                array<Byte>^ responseLength = BitConverter::GetBytes(response->Length);
                                connectedStream->Write(responseLength, 0, responseLength->Length);
                                connectedStream->Write(response, 0, response->Length);
                            }
                        }
                        else {
                            array<Byte>^ response = Encoding::UTF8->GetBytes("5");

                            for each (TcpClient ^ connectedClient in clients) {
                                NetworkStream^ connectedStream = connectedClient->GetStream();
                                array<Byte>^ responseLength = BitConverter::GetBytes(response->Length);
                                connectedStream->Write(responseLength, 0, responseLength->Length);
                                connectedStream->Write(response, 0, response->Length);
                            }
                        }
                        bdwordcount = 0;
                    }
                    else {
                        System::IO::File::AppendAllText(filePath, "\n" + modifiedData);
                        String^ saved = System::IO::File::ReadAllText("received_data.txt");

                        String^ responseStr = "1 " + saved;
                        array<Byte>^ response = Encoding::UTF8->GetBytes(responseStr);

                        // Send to all connected clients
                        for each (TcpClient ^ connectedClient in clients) {
                            NetworkStream^ connectedStream = connectedClient->GetStream();
                            array<Byte>^ responseLength = BitConverter::GetBytes(response->Length);
                            connectedStream->Write(responseLength, 0, responseLength->Length);
                            connectedStream->Write(response, 0, response->Length);
                        }
                    }
                }
            }
        }
    }
    catch (Exception^ ex) {
        Console::WriteLine("Error handling client: {0}", ex->Message);
    }
    finally {
        clients->Remove(client);
        client->Close();
    }
}



// Функція для обробки кожного клієнта
void HandleClient(SOCKET clientSocket) {
    char buffer[1024];
    int bytesRead;

    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytesRead] = '\0';  // Завершення рядка
        std::string data(buffer);

        // Обробка даних від клієнта
        if (data[0] == 'L') {
            // Логіка для логіну
            std::cout << "Login request received: " << data << std::endl;
        }
        else if (data[0] == 'S') {
            // Логіка для реєстрації
            std::cout << "Signup request received: " << data << std::endl;
        }
        else if (data[0] == 'T') {
            // Логіка для передачі повідомлень
            std::cout << "Message received: " << data << std::endl;
        }

        // Відповідь клієнту
        std::string response = "Server response: " + data;
        send(clientSocket, response.c_str(), response.length(), 0);
    }

    // Закриття з'єднання з клієнтом
    closesocket(clientSocket);
}

Task^ Server::ServerForm::HandleClientAsync(TcpClient^ client) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return Task::CompletedTask;
    }

    // Створення сокета
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        WSACleanup();
        return Task::CompletedTask;
    }

    // Налаштування параметрів сервера
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(54000);  // Порт для сервера
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Прив'язка сокета до адреси
    if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Bind failed" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return Task::CompletedTask;
    }

    // Слухання порту
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return Task::CompletedTask;
    }

    std::cout << "Server is listening on port 54000..." << std::endl;

    // Основний цикл обробки клієнтів
    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }

        // Створюємо новий потік для обробки кожного клієнта
        std::thread clientThread(HandleClient, clientSocket);
        clientThread.detach();  // Запускаємо потік та від'єднуємо його
    }

    // Закриваємо серверний сокет
    closesocket(serverSocket);
    WSACleanup();
    return Task::CompletedTask;
}
