#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <ctime>
#include <vector>
#include <thread>
#include <chrono>
//#include "protocol.h"
#include "smtp.h"
#include "pop3.h"
using namespace std;
using namespace std::chrono;


// Connect mail server: 
// "C:\Program Files\Java\jdk1.8.0_361\bin\java" -jar "E:\test-mail-server-1.0.jar" -s 3333 -p 4444 -m ./


struct MIME {
    bool is_Read = false;
    int numth = 0;
    string subject = "";
    string text_content = "";
    vector<string> file_content;
};

struct Config{
    int smtp_port = 0;
    int pop3_port = 0;
    string username;
    string password;
    string email;
    string mail_server;
    int autoload = 0;
};

struct Mail_box{
    vector<MIME> project;
    vector<MIME> important;
    vector<MIME> work;
    vector<MIME> spam;
    vector<MIME> inbox;
};

Config readFile(string filename){
    Config configuration;
    ifstream file(filename);
    if (file.is_open()){
        string line;
        while (getline(file, line)){
            if (line.find("SMTP") != string::npos){
                configuration.smtp_port = stoi(line.substr(line.find(":") + 2));
            }
            else if (line.find("POP3") != string::npos){
                configuration.pop3_port = stoi(line.substr(line.find(":") + 2));
            }
            else if (line.find("Username") != string::npos){
                string username = line.substr(line.find(":") + 3, line.find_last_of("\"") - line.find(":") - 3);
                int split_pos = username.find(" <");
                configuration.username = username.substr(0, split_pos);
                configuration.email = username.substr(split_pos + 2);
                configuration.email = configuration.email.substr(0, configuration.email.size() - 1);
            }
            else if (line.find("Password") != string::npos){
                configuration.password = line.substr(line.find(":") + 3, line.find_last_of("\"") - line.find(":") - 3);
            }
            else if (line.find("MailServer") != string::npos){
                configuration.mail_server = line.substr(line.find(":") + 3, line.find_last_of("\"") - line.find(":") - 3);
            }
            else if (line.find("Autoload") != string::npos){
                configuration.autoload = stoi(line.substr(line.find(":") + 2));
            }
        }
        file.close();
    }
    else{
        cout << "Unable to open file";
    }
    return configuration;
}

string getToday(){
    const auto now = system_clock::now();
    const time_t t_c = system_clock::to_time_t(now);
    string datetime = ctime(&t_c);
    return datetime;
}

vector<MIME> Split_mails(vector<string> datas){
    vector<MIME> result;
    for (int i = 0; i < datas.size(); i++){
        //cout << "aaaaaaaaaaaaaaaaaaaaa" << endl;
        MIME temp;
        temp.numth = i + 1;
        string data = datas[i];
        data = data.substr(data.find("\r\n") + 2);
        //cout << data << endl;
        int pos_quo = data.find("\"");
        string signal_end = data.substr(pos_quo + 1, data.find("\"", pos_quo + 1) - pos_quo - 1);
        //cout << signal_end;
        temp.subject = data.substr(0, data.find(signal_end + "\r\n") - 2);
        data = data.substr(data.find(signal_end + "\r\n") + signal_end.size() + 2);
        int pos = data.find(signal_end + "\r\n");
        if (pos == size_t(-1)){
            temp.text_content = data.substr(0, data.find(signal_end + "--") - 2);
            data = data.substr(data.find(signal_end + "--") + signal_end.size() + 2);
            result.push_back(temp);
            continue;
        }
        temp.text_content = data.substr(0, data.find(signal_end + "\r\n") - 2);
        //cout << temp.text_content;
        data = data.substr(data.find(signal_end + "\r\n") + signal_end.size() + 2);
        
        int find_end = data.find(signal_end + "--");
        if (find_end == size_t(-1)){
            result.push_back(temp);
        }
        else{
            while(1){
                int pos = data.find(signal_end + "\r\n");
                if (pos == size_t(-1))break;
                string each_file = data.substr(0, pos - 2);
                temp.file_content.push_back(each_file);
                data = data.substr(pos + signal_end.size() + 2);

            }
        }
        temp.file_content.push_back(data.substr(0, data.find(signal_end + "--") - 4));
        data = data.substr(data.find(signal_end + "--") + signal_end.size() + 2);
        //cout << temp.file_content[0];
        //cout << data;
        //break;
        result.push_back(temp);
    }
    return result;
}

namespace filter{
    bool is_Project(MIME mail){
    //cout << "sadf" << endl;
    bool result = false;
    string project_check[2] = {"ahihi@testing.com", "ahuu@testing.com"};
    string check_part = mail.subject;
    //cout << check_part << endl;
    string check_sign = "From: <";
    check_part = check_part.substr(check_part.find(check_sign));
    check_part = check_part.substr(check_sign.size() + 1);
    check_part = check_part.substr(0, check_part.find(">"));
    //cout << check_part;
    for (int i = 0; i < 2; i++){
        if (strcmp(check_part.c_str(), project_check[i].c_str()) == 0){
            result = true;
            break;
        }
        else continue;
    }
    //cout << result << endl;
    //cout << check_part << endl;
    return result;
    }

    bool is_Important(MIME mail){
        bool result = false;
        string project_check[2] = {"urgent", "ASAP"};
        string check_part = mail.subject;
        string check_sign = "Subject: ";
        check_part = check_part.substr(check_part.find(check_sign));
        check_part = check_part.substr(check_sign.size());
        check_part = check_part.substr(0, check_part.find("\r\n"));
        //cout << check_part << endl;
        //string t = "asdfsadf ASA p aaaaa";
        // cout << t.find("urgent");
        //t = t.substr(t.find("urgent"), 6);
        //cout << t;
        //cout << check_part;
        for (int i = 0; i < 2; i++){
            if (check_part.find(project_check[i]) < size_t(-1)){
                result = true;
                break;
            }
            else continue;
        }
        return result;
    }


    bool is_Work(MIME mail){
        bool result = false;
        string project_check[2] = {"report", "meeting"};
        string check_part = mail.text_content;
        string check_sign = "\r\n\r\n";
        check_part = check_part.substr(check_part.find(check_sign));
        check_part = check_part.substr(check_sign.size());
        check_part = check_part.substr(0, check_part.find("\r\n"));
        //string t = "asdfsadf report p aaaaa";
        // cout << t.find("urgent");
        //t = t.substr(t.find("urgent"), 6);
        //cout << t;
        //cout << check_part;
        for (int i = 0; i < 2; i++){
            if (check_part.find(project_check[i]) < size_t(-1)){
                result = true;
                break;
            }
            else continue;
        }
        return result;
    }

    bool is_Spam(MIME mail){
        bool result = false;
        string project_check[3] = {"virus", "hack", "crack"};
        string check_part_1 = mail.text_content;
        string check_sign = "\r\n\r\n";
        check_part_1 = check_part_1.substr(check_part_1.find(check_sign));
        check_part_1 = check_part_1.substr(check_sign.size());
        check_part_1 = check_part_1.substr(0, check_part_1.find("\r\n"));
        string check_part_2 = mail.subject;
        check_sign = "Subject: ";
        check_part_2 = check_part_2.substr(check_part_2.find(check_sign));
        check_part_2 = check_part_2.substr(check_sign.size());
        check_part_2 = check_part_2.substr(0, check_part_2.find("\r\n"));
        //string t = "asdfsadf hack p aaaaa";
        // cout << t.find("urgent");
        //t = t.substr(t.find("urgent"), 6);
        //cout << t;
        //cout << check_part;
        for (int i = 0; i < 3; i++){
            if (check_part_1.find(project_check[i]) < size_t(-1)){
                result = true;
                return result;
                break;
            }
            else continue;
        }
        for (int i = 0; i < 3; i++){
            if (check_part_2.find(project_check[i]) < size_t(-1)){
                result = true;
                return result;
                break;
            }
            else continue;
        }
        return result;
    }
}

Mail_box Filter_mail(vector<MIME> mails){
    Mail_box result;
    //cout << "asdf" << endl;
    for (int i = 0; i < mails.size(); i++){
        MIME temp = mails[i];
        //cout << temp.text_content;
        //cout << is_Project(temp) << endl;
        if (filter::is_Project(temp) == 1){
            //cout << "asdf" << endl;
            result.project.push_back(temp);
            continue;
        }
        else{
            if (filter::is_Important(temp) == 1){
                result.important.push_back(temp);
                continue;
            }
            else{
                if (filter::is_Work(temp) == 1){
                    //cout << "Datvip1234234reeeee" << endl;
                    result.work.push_back(temp);
                    continue;
                }
                else{
                    if (filter::is_Spam(temp) == 1){
                        //cout << "Datvip1234234qqqqqqqqqqq" << endl;
                        result.spam.push_back(temp);
                        continue;
                    }
                    else{
                        //cout << "Datvip1234234" << endl;
                        result.inbox.push_back(temp);
                    }
                }
            }
        }
    }
    return result;
};

void Send_email(Config configuration){
    //cout << "asdf" << endl;
    string date = getToday();
    SMTP smtp(configuration.mail_server, configuration.smtp_port);
    //cout << "Asdf" << endl;
    cout << "Input email you want to send to:" << endl;
    string to;
    //"ltm@hcmus.vn"
    cin >> to;
    int n_cc;
    while(1){
        cout << "Input the number of CC you want to send to:" << endl;
        cin >> n_cc;
        if (cin.fail()){
            cout << "Invalid input" << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(),'\n');
            continue;
        }
        else{
            break;
        }
    }
    vector<string> cc;
    for (int i = 0; i < n_cc; i++){
        string temp;
        cin >> temp;
        cc.push_back(temp);
    }
    //{"lolloine26@gmail.com", "phung26@gmail.com"}
    int n_bcc;
    while(1){
        cout << "Input the number of BCC you want to send to:" << endl;
        cin >> n_bcc;
        if (cin.fail()){
            cout << "Invalid input" << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(),'\n');
            continue;
        }
        else{
            break;
        }
    }
    //vector<string> bcc = {"example@gmail.com", "abcd@hcmus.com"};
    vector<string> bcc;
    for (int i = 0; i < n_bcc; i++){
        string temp;
        cin >> temp;
        bcc.push_back(temp);
    }
    smtp.login_authentication(configuration.email, configuration.password);
    string Topic;
    cout << "Input your Topic of mail: " << endl;
    cin.ignore();
    getline(cin, Topic);
    string Content;
    cout << "Input your Content of mail: " << endl;
    //cin.ignore();
    getline(cin, Content);
    //cout << Content << endl;
    //while(1);
    smtp.sendMail(configuration.email, to, cc, bcc, Topic, Content, date);

    //cout << "asdf" << endl;
    //smtp.sendEndOfMailMIME();
}

string from_text(string data){
    string check_sign = "From: <";
    data = data.substr(data.find(check_sign));
    data = data.substr(check_sign.size());
    data = data.substr(0, data.find(">"));
    return data;
}

string subject_text(string data){
    string check_sign = "Subject: ";
    data = data.substr(data.find(check_sign));
    data = data.substr(check_sign.size());
    data = data.substr(0, data.find("\r\n"));
    return data;
}

void Download_File(string data, string filename){
    //cout << data;
    ofstream File;
    File.open(filename, ios :: binary);
    File << data;
    File.close();
}

void Update_email(MIME mail, string message, Config configuration){
    //cout << message;
    SMTP smtp(configuration.mail_server, configuration.smtp_port);
    smtp.login_authentication(configuration.email, configuration.password);
    smtp.updateMail(message);

    POP3 pop3(configuration.mail_server, configuration.pop3_port);
    pop3.login(configuration.email, configuration.password);
    pop3.Delete_retrv(mail.numth);

}

void show_content(int mail_fd, vector<MIME> mails, vector<string> messages, Config configuration){
    // if (mails[mail_fd].subject.find("notRead") < size_t(-1)){
    //     Update_email(mails[mail_fd], messages[mails[mail_fd].numth - 1], configuration);
    // }
    //system("clear");
    string text_data = mails[mail_fd].text_content;
    string sign_check = "\r\n\r\n";
    text_data = text_data.substr(text_data.find(sign_check) + sign_check.size());
    cout << "Text content: " << endl;
    cout << text_data;
    cout << endl << endl;
    //cout << mails[mail_fd].file_content[0] << endl;
    if (mails[mail_fd].file_content.empty()){
        cout << "Input random string to return mail box" << endl;
        string command;
        cin >> command;
        int fd = atoi(command.c_str());
        if (fd >= 0)
            return;
    }
    if (mails[mail_fd].file_content.size() > 0){
        cout << "This mail has file, do you want to download" << endl;
        cout << "1. Yes" << endl;
        cout << "2. No" << endl;
        string command;
        cin >> command;
        int fd = atoi(command.c_str());
        if (fd == 0 || fd == 2){
            system("clear");
            return;
        }
        else{
            for (int i = 0; i < mails[mail_fd].file_content.size(); i++){
                string file_data = mails[mail_fd].file_content[i];
                sign_check = "filename=\"";
                string filename = file_data.substr(file_data.find(sign_check));
                filename = filename.substr(sign_check.size());
                filename = filename.substr(0, filename.find("\r\n") - 1);
                //cout << filename;
                sign_check = "\r\n\r\n";
                file_data = file_data.substr(file_data.find(sign_check) + sign_check.size());
                //cout << file_data;
                int pos = 0;
                //cout << file_data << endl;
                while(1){
                    //cout << "asdfasdf" << endl;
                    //cout << pos << endl;
                    pos = file_data.find("\r\n");
                    //cout << file_data;
                    //cout << pos;
                    //cout << "Asdfasdf" << endl;
                    //while(1);
                    //cout << pos << endl;
                    if (pos < 0){

                        break;
                    }
                    else{
                        string temp = file_data.substr(pos + 2);
                    //cout << temp << endl;
                        file_data = file_data.substr(0, pos);
                        file_data += temp;
                    }
                    
                    //cout << file_data << endl;
                    
                    //file_data = temp;
                }
                file_data = b64decode(file_data);
                //cout << file_data;
                //cout << file_data;
                Download_File(file_data, filename);
            }
        }
        
        
    }
    else return;
    return;
    //cout << text_data;
}

void Show_each_type(vector<MIME> mails, vector<string> messages, Config configuration){
    
    system("clear");
    //cout << "asdfasdf" << endl;
    //cout << mails[0].file_content[1];
    if (mails.size() == 0){
        //system("clear");
        cout << "No mails in this folder" << endl;
        cout << "Input random string to return mail box" << endl;
        string command;
        cin >> command;
        int fd = atoi(command.c_str());
        if (fd == 0)
            return;
    }
    else{
        while(true){
            //system("clear");
            cout << "List emails in this folder" << endl;
            for(int i = 0; i < mails.size(); i++){
                //cout << mails[i].numth << endl;
                cout << i + 1 << ". <" << from_text(mails[i].subject) << ">, <" << subject_text(mails[i].subject) << ">";
                cout << endl;
            }
            cout << endl;
            cout << "Input random string to return mail box" << endl;
            string command;
            cin >> command;
            int fd = atoi(command.c_str());
            if (fd == 0)
                return;
            else{
                show_content(fd - 1, mails, messages, configuration);
                continue;
            }
            
        }
    }
    return;
}

void Show_mails(Mail_box mail_box, vector<string> messages, Config configuration){
    system("clear");
    while(1){
        string sw;
        cout << "Select the type of mails you want to check" << endl;
        cout << "1. Inbox" << endl;
        cout << "2. Project" << endl;
        cout << "3. Important" << endl;
        cout << "4. Work" << endl;
        cout << "5. Spam" << endl;
        cout << "6. Return Menu" << endl;
        cin >> sw;
        int fd = atoi(sw.c_str());
        switch(fd){
            case 1:
                Show_each_type(mail_box.inbox, messages, configuration);
                break;
            
            case 2:
                Show_each_type(mail_box.project, messages, configuration);
                break;
            
            case 3:
                Show_each_type(mail_box.important, messages, configuration);
                break;
            
            case 4:
                Show_each_type(mail_box.work, messages, configuration);
                break;

            case 5:
                Show_each_type(mail_box.spam, messages, configuration);
                break;
            
            case 6:
                return;
                break;

            default:
                return; 
                break;
        }
        system("clear");
    }
}

void Update_POP3_each_time(Config temp, Mail_box& mail_box){
    while(true){
        this_thread::sleep_for(chrono::seconds(temp.autoload));
        POP3 pop3(temp.mail_server, temp.pop3_port);
        pop3.login(temp.email, temp.password);
        vector<string> messagesWithAttachments = pop3.retrieveMessagesWithAttachments();
        vector<MIME> mails = Split_mails(messagesWithAttachments);
        mail_box = Filter_mail(mails);
        //cout << mail_box.inbox.size();
        //cout << "asdsdf" << endl;
    }
    
    //return;
}

void Menu(){
    Config configuration = readFile("config.json"); 
    POP3 pop3(configuration.mail_server, configuration.pop3_port);
    pop3.login(configuration.email, configuration.password);
    vector<string> messagesWithAttachments = pop3.retrieveMessagesWithAttachments();
    vector<MIME> mails = Split_mails(messagesWithAttachments);
    Mail_box mail_box = Filter_mail(mails);
    //Mail_box mail_box;
    //cout << "cc" << endl;
    while(1){
        thread Task(Update_POP3_each_time, configuration, ref(mail_box));
        Task.detach();
        //cout << mail_box.spam.size();
        string sw;
        cout << "___________________________Menu___________________________" << endl;
        cout << "1. Send email" << endl;
        cout << "2. Show list of received email" << endl;
        cout << "3. Exit" << endl;
        cout << endl;
        cout << "Input your number of feature you want to use" << endl;
        cin >> sw;
        int fd = atoi(sw.c_str());
        switch(fd){
            case 1:
                Send_email(configuration);
                // pop3.login(configuration.email, configuration.password);
                // messagesWithAttachments = pop3.retrieveMessagesWithAttachments();
                // mails = Split_mails(messagesWithAttachments);
                // mail_box = Filter_mail(mails); 
                //cout << "asdfasdf" << endl;
                break;
            
            case 2:
                Show_mails(mail_box, messagesWithAttachments, configuration);
                break;
            
            case 3:
                //Task.detach();
                return;
                break;
            
            case -1:
                //Task.detach();
                return;
                break;

            default:
                //Task.detach();
                return;
                break;
        }
        //break;
        
        system("clear");
        
    }
}

int main(){
    //cout << "asdf" << endl;
    Menu();
    return 0;
}
