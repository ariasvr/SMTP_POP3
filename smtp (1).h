#pragma once

#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sstream>

#define MAX_FILE 3145728;


using namespace std;

static const char* B64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const int B64index[256] =
{
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  62, 63, 62, 62, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,  0,
    0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  63,
    0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
};

const std::string b64encode(const void* data, const size_t &len)
{
    std::string result((len + 2) / 3 * 4, '=');
    unsigned char *p = (unsigned  char*) data;
    char *str = &result[0];
    size_t j = 0, pad = len % 3;
    const size_t last = len - pad;

    for (size_t i = 0; i < last; i += 3)
    {
        int n = int(p[i]) << 16 | int(p[i + 1]) << 8 | p[i + 2];
        str[j++] = B64chars[n >> 18];
        str[j++] = B64chars[n >> 12 & 0x3F];
        str[j++] = B64chars[n >> 6 & 0x3F];
        str[j++] = B64chars[n & 0x3F];
    }
    if (pad)  /// Set padding
    {
        int n = --pad ? int(p[last]) << 8 | p[last + 1] : p[last];
        str[j++] = B64chars[pad ? n >> 10 & 0x3F : n >> 2];
        str[j++] = B64chars[pad ? n >> 4 & 0x03F : n << 4 & 0x3F];
        str[j++] = pad ? B64chars[n << 2 & 0x3F] : '=';
    }
    return result;
}

const std::string b64decode(const void* data, const size_t &len)
{
    if (len == 0) return "";

    unsigned char *p = (unsigned char*) data;
    size_t j = 0,
        pad1 = len % 4 || p[len - 1] == '=',
        pad2 = pad1 && (len % 4 > 2 || p[len - 2] != '=');
    const size_t last = (len - pad1) / 4 << 2;
    std::string result(last / 4 * 3 + pad1 + pad2, '\0');
    unsigned char *str = (unsigned char*) &result[0];

    for (size_t i = 0; i < last; i += 4)
    {
        int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 | B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
        str[j++] = n >> 16;
        str[j++] = n >> 8 & 0xFF;
        str[j++] = n & 0xFF;
    }
    if (pad1)
    {
        int n = B64index[p[last]] << 18 | B64index[p[last + 1]] << 12;
        str[j++] = n >> 16;
        if (pad2)
        {
            n |= B64index[p[last + 2]] << 6;
            str[j++] = n >> 8 & 0xFF;
        }
    }
    return result;
}

std::string b64encode(const std::string& str)
{
    return b64encode(str.c_str(), str.size());
}

std::string b64decode(const std::string& str64)
{
    return b64decode(str64.c_str(), str64.size());
}

vector<string> text_extension = {"txt", "pdf", "html", "doc", "docx", "json", "log", "rtf"};

string take_filename(string path){
    string result = path;
    
    while(1){
        int pos = result.find("/");
        if (pos == size_t(-1)){
            break;
        }
        else{
            result = result.substr(pos + 1); 
        }
    }
    return result;
}

string takeExtensionMode(string filename){
    string to_send = "text/plain";
    bool flag = true;
    string extension = filename.substr(filename.find_last_of('.') + 1);
    for (int i = 0; i < text_extension.size(); i++){
        if (extension.compare(text_extension[i]) == 0){
            return to_send;
        }
    }

    // if (find(text_extension.begin(), text_extension.end(), extension) != text_extension.end()){
    //     to_send = "application/" + extension;
    // }

    return "application/plain";
}

class SMTP{
    int sockfd;
    char buffer[1024];
    string boundary = "--abc123";

    void error(const char *msg){
        perror(msg);
        exit(0);
    }

    void sendCommand(string command){
        //cout << "sdf" << endl;
        //cout << command;
        bzero(buffer, 1024);
        //strcpy(buffer, command.c_str());
        if(send(sockfd, command.c_str(), command.size(), 0) < 0){
            error("Error writing to socket");
        }
    }

    void readResponse(){
        //cout << "asdfasdfasdfttttttttttttttttttttttt" << endl;
        bzero(buffer, 1024);
        if(recv(sockfd, buffer, 1024, 0) < 0){
            error("Error reading from socket");
        }
        //cout << buffer << endl;
    }

    int connect_server(string hostname, int port_num){
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
        //cout << "hello" << endl;
        
        return sockfd;
    }

    public:
        // Create a socket and connect to the server when creating SMTP object
        SMTP(string hostname="/", int port_num = 25){
            sockfd = connect_server(hostname, port_num);
            if (sockfd < 0){
                error("Error connecting to server");
            }
            else{
                sendCommand("HELO localhost\r\n\r\n");
                readResponse();
            }
        }

        void sendMail(string from, string to, vector<string> cc, vector<string> bcc, string topic, string content_text, string date){
            sendCommand("MAIL FROM: <" + from + ">\r\n");
            readResponse();

            sendCommand("RCPT TO: <" + to + ">\r\n");
            readResponse();

            for (int i = 0; i < cc.size(); i++){
                sendCommand("RCPT TO: <" + cc[i] + ">\r\n");
                readResponse();
            }

            for (int i = 0; i < bcc.size(); i++){
                sendCommand("RCPT TO: <" + bcc[i] + ">\r\n");
                readResponse();
            }
            sendHeaderMIME(from, to, cc, topic, date);
            sendMailTextMIME(content_text);
            //cout << "asdsadf" << endl;
            while(1){
                cout << "Do you want to sent file" << endl;
                cout << "1. Yes" << endl;
                cout << "2. No" << endl;
                string command;
                cin >> command;
                int fd = atoi(command.c_str());
                if (fd > 0){
                    if (fd == 1){
                        cout << "Input your number of files you want to sent" << endl;
                        string count_file;
                        cin >> count_file;
                        fd = atoi(count_file.c_str());
                        bool flag; //check file is big or not
                        if (fd > 0){
                            for (int i = 0; i < fd; i++){
                                string path;
                                cout << "Input the file path " << i + 1 << " : " << endl;
                                cin >> path;
                                flag = sendFileMIME(path, takeExtensionMode(path));
                                if (!flag){
                                    cout << "Invalid file (file's size is so big)" << endl;
                                    //i = i - 1 ;
                                    break;
                                };
                                if (i != fd - 1){
                                    continue;
                                }
                                else{
                                    sendEndOfMailMIME();
                                }
                                
                                //break;
                            }
                            if (flag == 0){
                                continue;
                            }
                            else{
                                break;
                            }
                            
                        }
                        else{
                            system("clear");
                            cout << "Invalid input" << endl;

                            continue;
                        }
                    }
                    else{
                        sendEndOfMailMIME();
                        break;
                    }   
                }
                else{
                    system("clear");
                    cout << "Invalid input" << endl;
                    continue;
                }
            }
            
            // sendCommand("DATA\r\n");
            // readResponse();

            // string content = "";
            // content += "MIME-Version: 1.0\r\n";
            // content += "From: <" + from + ">\r\n";
            // content += "To: <" + to + ">\r\n";
            // for (int i = 0; i < cc.size(); i++){
            //     content += "CC: <" + cc[i] + ">\r\n";
            // }
            // content += "Subject: " + subject + "\r\n";
            // content += "Date: " + date + " -0400\r\n\r\n";
            // content += message + "\r\n";
            // sendCommand(content);
            // readResponse();
        }

        void updateMail(string message){
            string temp = message;
            string end_line = ">\r\n";
            temp = temp.substr(temp.find("From: <") + 7);
            string from = temp.substr(0, temp.find(end_line));
            cout << from << endl;
            temp = temp.substr(temp.find(end_line) + end_line.size());
            //cout << temp << endl;
            temp = temp.substr(temp.find("To: <") + 5);
            string to = temp.substr(0, temp.find(">"));
            cout << to << endl;
            temp = temp.substr(temp.find(end_line) + end_line.size());
            sendCommand("MAIL FROM: <" + from + ">\r\n");
            readResponse();

            sendCommand("RCPT TO: <" + to + ">\r\n");
            readResponse();

            sendCommand("DATA\r\n");
            readResponse();

            string signature = "notRead";
            size_t index = message.find(signature);

            message.replace(index, signature.size(), "Read");


            int max = message.size();
            int size_count = 0;

            while(true){
                bzero(buffer, 1024);
                //cout << max << endl << size_count << endl;
                if (max - size_count <= 1000){
                    //cout << data << endl;
                    sendCommand(message);
                    size_count = max;
                    break;
                }
                //cout << buffer;
                // cout << "sdfs" << endl;
                // cout << size_count << endl;
                // cout << "dddddddddd" << endl;
                // cout << max << endl;
                // size_count += 1000;
                size_count += 1000;
                //cout << data.substr(0, 1000);
                sendCommand(message.substr(0, 1000));
                message = message.substr(1000);
            }
            sendCommand("QUIT\r\n");
            //cout << buffer << endl;
            //cout << "asdfdsafasdfasdf" << endl;
            readResponse();
            //cout << buffer << endl;
            bzero(buffer, 1024);
            //bzero(buffer, 1024);
            //cout << "asdfasdf" << endl;
            closeConnection();
            //sendEndOfMailMIME();
            //vector<string> cc;
            // while(message.find("CC: <") < size_t(-1)){
            //     cc.append(temp.substr(message.find("CC: <") + 1, temp.find(">")));
            //     temp = temp.substr(temp.find(end_line) + end_line.size());
            // }
            
        }

        //TEXT ONLY
        // void sendHeader(string from, string to, vector<string> cc, string subject, string date){
        //     sendCommand("DATA\r\n");
        //     readResponse();

        //     string content = "";
        //     content += "MIME-Version: 1.0\r\n";
        //     content += "From: <" + from + ">\r\n";
        //     content += "To: <" + to + ">\r\n";
        //     for (int i = 0; i < cc.size(); i++){
        //         content += "CC: <" + cc[i] + ">\r\n";
        //     }
        //     content += "Subject: " + subject + "\r\n";
        //     content += "Date: " + date + " -0400\r\n\r\n";
        //     sendCommand(content);
        //     readResponse();
        // }

        void sendMailText(string message){
            sendCommand(message + "\r\n");
            readResponse();
        }

        void sendEndOfMailText(){
            sendCommand("\r\n.\r\n");
            readResponse();

            sendCommand("QUIT\r\n");
            readResponse();
        }


        // MIME FORMAT
        void sendHeaderMIME(string from, string to, vector<string> cc, string subject, string date){
            sendCommand("DATA\r\n");
            readResponse();

            string content = "";
            content += "MIME-Version: 1.0\r\n";
            content += "Content-Type: multipart/mixed; boundary=\"" + boundary + "\"\r\n";
            content += "From: <" + from + ">\r\n";
            content += "To: <" + to + ">\r\n";
            for (int i = 0; i < cc.size(); i++){
                content += "CC: <" + cc[i] + ">\r\n";
            }
            content += "Subject: " + subject + "\r\n";
            content += "Date: " + date + " -0400\r\n\r\n";
            content += "notRead\r\n";
            sendCommand(content);
            //readResponse();            
        }

        void sendMailTextMIME(string message){
            string content = "";
            content += boundary + "\r\n";
            content += "Content-Type: text/plain; charset=\"utf-8\"\r\n";
            content += "Content-Transfer-Encoding: 7bit\r\n\r\n";
            content += message + "\r\n";
            sendCommand(content);
            //readResponse();
        }

        bool sendFileMIME(string path_file, string extension){
            // Initialize the file
            //cout << "asdfdddddddd" << endl;
            ifstream file(path_file);
            while (!file){
                perror("Error opening file");
                string path;
                cout << "Input again path to of file" << endl;
                cin >> path;
                path_file = path;
                file.open(path_file);
            }
            file.seekg(0,ios_base::end);
            int size = file.tellg();
            int max_size = MAX_FILE;
            if (size > max_size){
                return false;
            }
            file.seekg(0);
            string content = "";
            content += "\r\n" + boundary + "\r\n";
            content += "Content-Type: " + extension +"; name=\"" + take_filename(path_file) + "\"\r\n";
            content += "Content-Transfer-Encoding: base64\r\n";
            content += "Content-Disposition: attachment; filename=\"" + take_filename(path_file) + "\"\r\n\r\n";
            sendCommand(content);
            //cout << content << endl;
            // cout<<"000000000"<<endl;
            // readResponse();
            // cout<<"1111111"<<endl;

            // Send file content
            // ifstream file(path_file);
            // while (!file){
            //     perror("Error opening file");
            //     string path;
            //     cout << "Input again path to of file" << endl;
            //     cin >> path;
            //     path_file = path;
            //     file.open(path_file);
            // }

            stringstream ss;
            ss << file.rdbuf();

            string data = ss.str();
            //cout << data << endl;
            data = b64encode(data);
            //cout << data.size();
            int max = data.size();
            int size_count = 0;
            //cout << max << endl;
            //cout << data;
            while(true){
                bzero(buffer, 1024);
                //cout << max << endl << size_count << endl;
                if (max - size_count <= 1000){
                    //cout << data << endl;
                    sendCommand(data + "\r\n");
                    size_count = max;
                    break;
                }
                //cout << buffer;
                // cout << "sdfs" << endl;
                // cout << size_count << endl;
                // cout << "dddddddddd" << endl;
                // cout << max << endl;
                // size_count += 1000;
                size_count += 1000;
                //cout << data.substr(0, 1000);
                sendCommand(data.substr(0, 1000) + "\r\n");
                data = data.substr(1000);
            }
            file.close();
            //cout << data;
            //sendCommand(data);
            //data = data.substr(3011500);
            //cout << data;
            //cout << size_count;
            //while()
            // char* file_content = new char[1000];
            // int read_size = 0;
            // // fread(file_content, 0, 1024, file);
            // // cout<<file_content<<endl;
            // // sendCommand(base64_encode(file_content, 1024) + "\r\n");
            // // readResponse(); 
            // while(true){
            //     //cout << "asdf" << endl;
            //     bzero(file_content, 1000);
            //     if (read_size < file_size){
                    
            //         if (file_size - read_size >= 1000){
            //             fread(file_content, 1, 1000, file);
            //             read_size += 1000;
            //             string temp = file_content;
            //             cout << temp << endl;
            //             temp += "\r\n";
            //             //cout << "asdf" << endl;
            //             sendCommand(temp);
            //         }
            //         else{
            //             fread(file_content, 1, file_size - read_size, file);
            //             string temp = file_content;
            //             cout << temp << endl;
            //             temp += "\r\n";
            //             sendCommand(temp);
            //             read_size = file_size;
            //         }
            //         //cout << file_content << endl;
            //         //cout << read_size << endl;
            //     }
            //     else{
            //         break;
            //     }
            // } 
            // bzero(file_content, 1000);
            //cout << "asdf" << endl;
            // while (read_size < file_size){
            //     cout << "File_size" << endl;
            //     cout << read_size << endl;
            //     bzero(file_content, 1000);
            //     if (file_size - read_size > 1000){
            //         fread(file_content, 1, 1000, file);
            //         read_size += 1000;
            //         string temp = base64_encode(file_content, 1000);
            //         temp += "\r\n";
            //         //sendCommand(base64_encode(file_content, 1021));
            //         sendCommand(temp);
            //         //readResponse();
            //     }
            //     else{
            //         fread(file_content, 1, file_size - read_size, file);
            //         //sendCommand(base64_encode(file_content, file_size - read_size));
            //         string temp = base64_encode(file_content, file_size - read_size);
            //         temp += "\r\n";
            //         sendCommand(temp);
            //         //readResponse();
            //         read_size = file_size;
            //     }
            //     //cout << file_content << endl;
            //     //cout << read_size << endl;
            // }
            //cout << "asdf" << endl;
            //cout << file_size << endl;
            //bzero(file_content, 1024);
            //cout << buffer << endl;
            //fclose(file);
            //bzero(file_content, 1000);
            //free(file_content);
            //cout << file_content << endl;
            //cout << "asdfasdfasdfadsfasdfasdfasdfasdf" << endl;
            return true;
        }

        void sendEndOfMailMIME(){
            //cout << buffer << endl;
            sendCommand("\r\n--abc123--\r\n.\r\n");
            //cout << buffer << endl;
            readResponse();
            //cout << buffer << endl;
            sendCommand("QUIT\r\n");
            //cout << buffer << endl;
            //cout << "asdfdsafasdfasdf" << endl;
            readResponse();
            //cout << buffer << endl;
            bzero(buffer, 1024);
            //bzero(buffer, 1024);
            //cout << "asdfasdf" << endl;
            closeConnection();
        }

        void closeConnection(){
            close(sockfd);
        }

        void login_authentication(string username, string password){
            sendCommand("AUTH LOGIN\r\n");
            readResponse();

            string username_64 =  b64encode(username);
            sendCommand(username_64 + "\r\n");
            readResponse();

            string password_64 = b64encode(password);
            sendCommand(password_64 + "\r\n");
            readResponse();
        }

        ~SMTP(){
            close(sockfd);
        }
};