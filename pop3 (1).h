#pragma once
#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <vector>
#include <sstream>
#include <fstream>

#include "smtp.h"
using namespace std;

// static const char* B64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// static const int B64index[256] =
// {
//     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
//     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
//     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  62, 63, 62, 62, 63,
//     52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,  0,
//     0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
//     15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  63,
//     0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
//     41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
// };

// const std::string b64encode(const void* data, const size_t &len)
// {
//     std::string result((len + 2) / 3 * 4, '=');
//     unsigned char *p = (unsigned  char*) data;
//     char *str = &result[0];
//     size_t j = 0, pad = len % 3;
//     const size_t last = len - pad;

//     for (size_t i = 0; i < last; i += 3)
//     {
//         int n = int(p[i]) << 16 | int(p[i + 1]) << 8 | p[i + 2];
//         str[j++] = B64chars[n >> 18];
//         str[j++] = B64chars[n >> 12 & 0x3F];
//         str[j++] = B64chars[n >> 6 & 0x3F];
//         str[j++] = B64chars[n & 0x3F];
//     }
//     if (pad)  /// Set padding
//     {
//         int n = --pad ? int(p[last]) << 8 | p[last + 1] : p[last];
//         str[j++] = B64chars[pad ? n >> 10 & 0x3F : n >> 2];
//         str[j++] = B64chars[pad ? n >> 4 & 0x03F : n << 4 & 0x3F];
//         str[j++] = pad ? B64chars[n << 2 & 0x3F] : '=';
//     }
//     return result;
// }

// const std::string b64decode(const void* data, const size_t &len)
// {
//     if (len == 0) return "";

//     unsigned char *p = (unsigned char*) data;
//     size_t j = 0,
//         pad1 = len % 4 || p[len - 1] == '=',
//         pad2 = pad1 && (len % 4 > 2 || p[len - 2] != '=');
//     const size_t last = (len - pad1) / 4 << 2;
//     std::string result(last / 4 * 3 + pad1 + pad2, '\0');
//     unsigned char *str = (unsigned char*) &result[0];

//     for (size_t i = 0; i < last; i += 4)
//     {
//         int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 | B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
//         str[j++] = n >> 16;
//         str[j++] = n >> 8 & 0xFF;
//         str[j++] = n & 0xFF;
//     }
//     if (pad1)
//     {
//         int n = B64index[p[last]] << 18 | B64index[p[last + 1]] << 12;
//         str[j++] = n >> 16;
//         if (pad2)
//         {
//             n |= B64index[p[last + 2]] << 6;
//             str[j++] = n >> 8 & 0xFF;
//         }
//     }
//     return result;
// }

// std::string b64encode(const std::string& str)
// {
//     return b64encode(str.c_str(), str.size());
// }

// std::string b64decode(const std::string& str64)
// {
//     return b64decode(str64.c_str(), str64.size());
// }

class POP3 {
    int sockfd;
    char buffer[1024];

    void error(const char *msg) {
        perror(msg);
        exit(0);
    }

    void sendCommand(string command) {
        bzero(buffer, 1024);
        strcpy(buffer, command.c_str());
        if(send(sockfd, buffer, strlen(buffer), 0) < 0){
            error("Error writing to socket");
        }
    }

    void readResponse(int length) {
        bzero(buffer, 1024);
        if(recv(sockfd, buffer, length, 0) < 0){
            error("Error reading from socket");
        }

    }

    int connect_server(string hostname, int port_num) {
        struct hostent* h_ip = gethostbyname(hostname.c_str());
        if (h_ip == NULL){
            perror("Error getting IP address of server");
            exit(1);
        }

        sockaddr_in serverAddr;
        bzero(&serverAddr, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port_num);
        bcopy(h_ip->h_addr, &serverAddr.sin_addr.s_addr, h_ip->h_length);

        sockaddr &serverAddrCast = (sockaddr&)serverAddr;

        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0){
            perror("Error creating socket");
            exit(1);
        }

        if (connect(sockfd, &serverAddrCast, sizeof(serverAddr)) < 0){
            return -1;
        }
        
        return sockfd;
    }

public:
    // Create a socket and connect to the server when creating POP3 object
    POP3(string hostname="/", int port_num = 110){
        sockfd = connect_server(hostname, port_num);
        if (sockfd < 0){
            error("Error connecting to server");
        }
        else{
            readResponse(1024); // Read welcome message from server
        }
    }

    void login(string username, string password){
        sendCommand("USER " + username + "\r\n");
        readResponse(1024);

        sendCommand("PASS " + password + "\r\n");
        readResponse(1024);
    }

    void quit(){
        sendCommand("QUIT\r\n");
        readResponse(1024);
        close(sockfd);
    }

    vector<string> retrieveMessagesWithAttachments() {
        vector<string> vector_string;
        vector<string> messageNumbers = listMessages();
        vector<string> fileNumbers = listFiles();
        for (int i = 0; i < messageNumbers.size(); i++){
            int num_mess = messageNumbers[i].find(" ");
            string temp;
            //pair<string, string> Pair;
            //Pair.first = messageNumbers[i].substr(0, num_mess);
            //cout << stoi(messageNumbers[i].substr(0, num_mess)) << endl;
            temp = retrieveMessageContent(stoi(messageNumbers[i].substr(0, num_mess)), stoi(messageNumbers[i].substr(num_mess)));
            //cout << temp << endl;
            vector_string.push_back(temp);
            //break;
        }
        quit();
        //cout << messagesWithAttachments[1].second << endl;
        //cout << messageNumbers[0] << endl;
        //for (const auto& msgNumber : messageNumbers) {

            //string messageContent = retrieveMessageContent(stoi(msgNumber));
            //vector<string> attachments = extractAttachments(messageContent);
            // messagesWithAttachments.push_back({msgNumber, messageContent});
            // for (const auto& attachment : attachments) {
            //     messagesWithAttachments.push_back({msgNumber, attachment});
            // }
        //}
        //cout << messageNumbers.size() << endl;
        // for (int i = 0; i < vector_string.size(); i++){
        //     //Pair.first = messageNumbers[i].substr(0, num_mess);
        //     //cout << vector_string[i] << endl;
        //     //Pair.second = retrieveMessageContent(stoi(Pair.first));
        //     //cout << messagesWithAttachments[i].second << endl;

        // }
        return vector_string;
    }

    void Delete_retrv(int fd_retrv){
        sendCommand("DELE " + to_string(fd_retrv) + "\r\n");
        quit();
    }

    ~POP3(){
        close(sockfd);
    }

private:
    vector<string> listMessages(){
        vector<string> messages;
        sendCommand("LIST\r\n");
        readResponse(1024);
        string data = buffer;
        int pos  = data.find("\r\n");
        data = data.substr(pos + 1);
        pos = data.find(".");
        int count = 0;
        //cout << data;
        //int end_num = data.find("\r\n");
        //cout << end_num << endl;
        //cout << data.substr(1, end_num + 1);
        while(count <= pos){
            //cout << "Count" << endl;
            //cout << count << endl;
            //cout << "Pos" << endl;
            //cout << pos << endl;
            int end_num = data.find("\r\n");
            //cout << end_num;
            //cout << data.substr(0, end_num) << endl;
            string mes = data.substr(1, end_num + 1);
            mes = mes.substr(0, mes.size() - 2);
            messages.push_back(mes);
            data = data.substr(end_num + 1);
            //cout << messages[count] << endl;
            //cout << data;
            count += (end_num + 1);
            //break;
        };
        //vector<int>::iterator it = messages.end();
        messages.erase(messages.end());
        // for (int i = 0; i < messages.size(); i++){
        //     cout << messages[i] << endl;
        // }
        // Parse the response to extract message numbers and sizes
        // Append each message number to the vector
        return messages;
    }

    // vector<string> listFiles(){
    //     vector<string> Files;
    //     sendCommand("UIDL\r\n");
    //     readResponse(1024);
    //     // Parse the response to extract message numbers and sizes
    //     // Append each message number to the vector
    //     return Files;
    // }
     vector<string> listFiles(){
        vector<string> messages;
        sendCommand("UIDL\r\n");
        string data = "";
        readResponse(1024);
        // while(!(buffer[strlen(buffer) - 1] == '.')){
        //     cout << buffer;
        //     //readResponse(1024);
        //     data += buffer;
        //     readResponse(1024);
        // }
        data += buffer;
        //string data = buffer;
        
        int pos  = data.find("\r\n");
        data = data.substr(pos + 2);
        //cout << data;
        pos = data.find("\r\n.");
        //cout << pos << endl;
        int count = 0;
        //cout << data;
        //int end_num = data.find("\r\n");
        //cout << end_num << endl;
        //cout << data.substr(1, end_num + 1);
        while(count <= pos){
            
            // cout << "Count" << endl;
            // cout << count << endl;
            // cout << "Pos" << endl;
            // cout << pos << endl;
            int end_num = data.find("\r\n");
            //cout << end_num;
            //cout << data.substr(1, end_num + 1) << endl;
            string mes = data.substr(0, end_num);
            //cout << mes << endl;
            //mes = mes.substr(0, mes.size() - 2);
            messages.push_back(mes);
            data = data.substr(end_num + 2);
            //cout << messages[count] << endl;
            //cout << data;
            count += (end_num + 2);
            //break;
            //break;
        };


        //cout << messages.size() << endl;
        //vector<int>::iterator it = messages.end();
        //messages.erase(messages.end());
        // for (int i = 0; i < messages.size(); i++){
        //     cout << messages[i] << endl;
        // }
        // Parse the response to extract message numbers and sizes
        // Append each message number to the vector
        return messages;
    }


    string retrieveMessageContent(int msg_number, int size_data) {
        //string messageContent;
        //cout << "asdf" << endl;
        //cout << size_data << endl;
        //cout << to_string(size_data).size() + 9 << endl;
        //cout << size_data << endl;
        //cout << to_string(size_data).size() << endl;
        size_data = size_data + to_string(size_data).size() + 9;
        sendCommand("RETR " + to_string(msg_number) + "\r\n");
        string data = "";
        int data_count = 0;
        int size_temp = 1000;
        while (1) {
            //cout << buffer;
            bzero(buffer, 1024);
            if (size_data - data_count <= 1000)size_temp = size_data - data_count;
            //recv(sockfd, buffer, sizeof(buffer), 0)
            if (recv(sockfd, buffer, size_temp, MSG_PEEK) != size_temp)continue;
            else{
                recv(sockfd, buffer, size_temp, 0);
            }
            //cout << data_count << endl;
            //cout << size_data << endl;
            //recv(sockfd, buffer, size_temp, 0);
            //cout << nDataLength << endl;
            data_count += 1000;
            data.append(buffer, 1000);
            
            //data.append(buffer, nDataLength);
            //cout << data;
            //cout << data_count << endl;
            //cout << size_data << endl;
            //bzero(buffer, 1024);
            if (data_count >= size_data)break; 
        }
        //cout << "cc" << endl;

        //data = b64decode(data);
        //cout << "cc" << endl;
        //cout << data << endl;
        //cout << "asdfasdf" << endl;
        //cout << data << endl;
        //cout << data.find("StackExchange.ga");
        //cout << nDataLength << endl;
        //cout << data << endl;
        // while(true){
            
        //     //readResponse(1024);
        //     readResponse(1024);
        //     cout  << buffer;
        //     data += buffer;
        //     //cout << buffer << endl;
        //     //cout << data.substr(data.size() - 3) << endl;
        //     //string comp = data.substr(data.size() - 3);
        //     //cout << comp << endl;
        //     //string end_sign = ".\r\n";
        //     //cout << end_sign.size() << endl;
        //     //cout << strcmp(comp.c_str(), ".\r\n") << endl;
        //     string temp;
        //     temp = buffer;
        //     if (temp.find("\r\n.\r\n") < size_t(-1)){
        //         break;
        //     }
                
        //     //cout << "String" << endl;
        //     //cout << data[data.size() - 1] << endl;
        //     //cout << "Buffer << endflkas" << endl;
        //     //cout << buffer << endl;
        // }
        //data += buffer;
        //cout << data << endl;
        //cout << data << endl;
        // Capture the response which contains the message content
        //messageContent = readMessageContent();
        return data;
    }

    // string readMessageContent() {
    //     string messageContent;
    //     char tempBuffer[1024];
    //     int bytesReceived;

    //     // Read message content from socket until the end of message marker is encountered
    //     while ((bytesReceived = recv(sockfd, tempBuffer, 1024, 0)) > 0) {
    //         tempBuffer[bytesReceived] = '\0';
    //         messageContent += tempBuffer;
    //         // Check for end of message marker (e.g., ".\r\n")
    //         if (messageContent.find("\r\n.\r\n") != string::npos) {
    //             break;
    //         }
    //     }

    //     if (bytesReceived < 0) {
    //         error("Error reading from socket");
    //     }

    //     return messageContent;
    // }

    vector<string> extractAttachments(const string &messageContent) {
        vector<string> attachments;
        stringstream ss(messageContent);
        string line;

        // Skip headers
        while (getline(ss, line) && line != "\r") {}

        // Extract attachments (if any)
        while (getline(ss, line)) {
            if (line.substr(0, 2) == "--") {
                // Start of attachment section
                string attachmentContent;
                while (getline(ss, line) && line.substr(0, 2) != "--") {
                    attachmentContent += line + "\n";
                }
                attachments.push_back(attachmentContent);
            }
        }

        return attachments;
    }

    

};
