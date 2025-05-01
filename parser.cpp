#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>

using namespace std;

enum tag {
    作文,
    技術,
    ニュース
};

class post {
    public:
    post(string a, string b, string c, tag d, string e) {
        file_name = a;
        title = b;
        date = c;
        tags = d;
        body = e;
    }
    string get_file_name() { return file_name; }
    string get_title() { return title; }
    string get_date() { return date; }
    tag get_tags() { return tags; }
    string get_body() { return body; }

    private:
    string file_name = "";
    string title = "";
    string date = "";
    tag tags = 作文;
    string body = "";
};

// txtファイル + テンプレート -> HTML
void analyze_file(ifstream const &f);
string create_post(post &mypost, string tmplt);
// analyze_file用のファンクション
string find_file_name(string const &s);
string find_title(string const &s);
string find_date(string const &s);
string find_tag(string const &s);
string find_body(string const &s);
string txtformat_to_htmlformat(string s);
string update_catalog(string s);
void update_writting_catalog(string &s);
void update_tech_catalog(string &s);
void update_news_catalog(string &s);
bool sort_recent(post a, post b);
tag str_to_tag(string const &s);
string tag_to_str(tag t);
string which_folder(tag t);

// グローバル変数
vector<post> posts;

int main() {

    cin.tie(NULL);
    cout.tie(NULL);
    ios::sync_with_stdio(false);

    // フォルダーを巡って分析
    for (auto& i : filesystem::directory_iterator("./original")) {
        for (auto& j : filesystem::directory_iterator(i.path())) {
            if (j.is_regular_file()) {
                ifstream f(j.path());
                if (f.is_open()) {
                    analyze_file(f);
                    f.close();
                }
            }
        }
    }

    // テンプレートをオープン
    ifstream tmp("./post_template.html");
    stringstream sstream;
    sstream << tmp.rdbuf();

    // テンプレート HTML -> TXT
    string tmplt = sstream.str();

    sstream.str("");
    sstream.clear();
    tmp.close();

    // 毎回全体データ更新。既存データ削除
    for (auto& i : filesystem::directory_iterator("./post")) {
        for (auto& j : filesystem::directory_iterator(i.path())) {
            filesystem::remove(j.path());
        }
    }

    // post/writting 新データ装入
    for (int i = 0; i < posts.size(); i++) {
        string new_post = create_post(posts[i], tmplt);
        string new_title;

        new_title = "./post/" + which_folder(posts[i].get_tags()) + "/"
            + posts[i].get_file_name() + ".html";

        ofstream f(new_title);

        if (f.is_open()) {
            f << new_post;
            f.close();
        } else { cout << "[ofstream] ディレクトリのアクセス不可能\n"; }
    }

    // データ更新をブログに反映
    tmp.open("./index.html");
    sstream << tmp.rdbuf();
    string new_blog = sstream.str();

    sstream.str("");
    sstream.clear();
    tmp.close();

    // 最新投稿が最上段へ
    sort(posts.begin(), posts.end(), sort_recent);

    new_blog = update_catalog(new_blog);

    ofstream update("index.html", iostream::trunc);
    if (update.is_open()) {
        update << new_blog;
        update.close();
    }

    cout << "All done, clossing program.";
    exit(0);

}

void analyze_file(ifstream const &f) {
    string str = "";
    stringstream sstream;
    sstream << f.rdbuf();
    str = sstream.str(); // ファイル全体のテキスト

    string file_name = find_file_name(str);
    string title = find_title(str);
    string date = find_date(str);

    string t = find_tag(str);
    tag new_tag = str_to_tag(t);

    string body = find_body(str);

    post mypost = { file_name, title, date, new_tag, body };
    posts.push_back(mypost);
}

string create_post(post &mypost, string tmplt) {
    // <header>の<title>の{{title}}
    tmplt.replace(tmplt.find("{{title}}"), 9, mypost.get_title());
    // <main>の{{title}}
    tmplt.replace(tmplt.find("{{title}}"), 9, mypost.get_title());

    // 20250101 -> 2025-01-01
    string date = mypost.get_date();
    date.insert(4, "-");
    date.insert(7, "-");

    tmplt.replace(tmplt.find("{{date}}"), 8, date);
    tmplt.replace(tmplt.find("{{tags}}"), 8, tag_to_str(mypost.get_tags()));

    string body = txtformat_to_htmlformat(mypost.get_body());
    tmplt.replace(tmplt.find("{{body}}"), 8, body);
    
    return tmplt;
}

string find_file_name(string const &s) {
    string file_name = "";
    int target = s.find("@file_name: ") + 12;
    while (s[target] != '\n') {
        file_name.push_back(s[target]);
        target++;
    }
    return file_name;
}

string find_title(string const &s) {
    string title = "";
    int target = s.find("@title: ") + 8;
    while (s[target] != '\n') {
        title.push_back(s[target]);
        target++;
    }
    return title;
}

string find_date(string const &s) {
    string date = "";
    int target = s.find("@date: ") + 7;
    while (s[target] != '\n') {
        date.push_back(s[target]);
        target++;
    }
    return date;
}

string find_tag(string const &s) {
    string tag = "";
    int target = s.find("@tags: ") + 7;
    while (s[target] != '\n') {
        tag.push_back(s[target]);
        target++;
    }
    return tag;
}

string find_body(string const &s) {
    string body = "";
    int target = s.find_last_of("@");
    while (s[target] != '\n' && s[target] != '\r') {
        target++;
    }
    body = s.substr(target);
    return body;
}

bool sort_recent(post a, post b) {
    return stoi(a.get_date()) > stoi(b.get_date());
}

string txtformat_to_htmlformat(string s) {
    // \n -> <br>
    size_t pos = 0;
    while ((pos = s.find('\n', pos)) != std::string::npos) {
        s.replace(pos, 1, "<br>");
        pos += 4;
    }
    return s;
}

string update_catalog(string new_blog) {
    update_writting_catalog(new_blog);
    update_tech_catalog(new_blog);
    update_news_catalog(new_blog);
    return new_blog;
}

void update_writting_catalog(string &s) {
    int writting_idx_start = s.find("<!--writting_auto_add_start-->") + 30;
    int writting_idx_end = s.find("<!--writting_auto_add_end-->");

    // 既存データ削除
    s.erase(writting_idx_start, writting_idx_end - writting_idx_start);

    // データ更新をブログに反映
    string writting_catalog;
    for (int i = 0; i < posts.size(); i++) {
        if (posts[i].get_tags() == 作文) {
            string title = posts[i].get_title();
            string file_name = posts[i].get_file_name();
            writting_catalog += "<li><a href=\"#\" onclick=\"load_post('";
            writting_catalog +=  file_name + "', '" + tag_to_str(posts[i].get_tags());
            writting_catalog += "')\">" + title;
            writting_catalog += "</a></li>\n";
        }
    }
    s.insert(writting_idx_start, writting_catalog);
}

void update_tech_catalog(string &s) {
    int tech_idx_start = s.find("<!--tech_auto_add_start-->") + 26;
    int tech_idx_end = s.find("<!--tech_auto_add_end-->");

    // 既存データ削除
    s.erase(tech_idx_start, tech_idx_end - tech_idx_start);

    // データ更新をブログに反映
    string tech_catalog;
    for (int i = 0; i < posts.size(); i++) {
        if (posts[i].get_tags() == 技術) {
            string title = posts[i].get_title();
            string file_name = posts[i].get_file_name();
            string tag = tag_to_str(posts[i].get_tags());
            tech_catalog += "<li><a href=\"#\" onclick=\"load_post('";
            tech_catalog +=  file_name + "', '" + tag;
            tech_catalog += "')\">" + title;
            tech_catalog += "</a></li>\n";
        }
    }
    s.insert(tech_idx_start, tech_catalog);
}

void update_news_catalog(string &s) {
    int news_idx_start = s.find("<!--news_auto_add_start-->") + 26;
    int news_idx_end = s.find("<!--news_auto_add_end-->");

    // 既存データ削除
    s.erase(news_idx_start, news_idx_end - news_idx_start);

    // データ更新をブログに反映
    string news_catalog;
    for (int i = 0; i < posts.size(); i++) {
        if (tag_to_str(posts[i].get_tags()) =="ニュース") {
            string title = posts[i].get_title();
            string file_name = posts[i].get_file_name();
            string tag = tag_to_str(posts[i].get_tags());
            news_catalog += "<li><a href=\"#\" onclick=\"load_post('";
            news_catalog +=  file_name + "', '" + tag;
            news_catalog += "')\">" + title;
            news_catalog += "</a></li>\n";
        }
    }
    s.insert(news_idx_start, news_catalog);
}

tag str_to_tag(string const &s) {
    if (s == "作文") return 作文;
    else if (s == "技術") return 技術;
    else if (s == "ニュース") return ニュース;
    else return 作文;
}

string tag_to_str(tag t) {
    switch (t) {
        case 作文:  return "作文";
        case 技術:  return "技術";
        case ニュース:  return "ニュース";
        default:    return "作文";
    }
}

string which_folder(tag t) {
    switch (t) {
        case 作文:  return "writting";
        case 技術:  return "tech";
        case ニュース:  return "news";
        default:    return "writting";
    }
}