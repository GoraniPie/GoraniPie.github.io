#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>

using namespace std;

class post {
    public:
    post(string a, string b, string c, string d, string e) {
        file_name = a;
        title = b;
        date = c;
        tags = d;
        body = e;
    }
    string get_file_name() { return file_name; }
    string get_title() { return title; }
    string get_date() { return date; }
    string get_tags() { return tags; }
    string get_body() { return body; }

    private:
    string file_name = "";
    string title = "";
    string date = "";
    string tags = "";
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
bool sort_recent(post a, post b);

// グローバル変数
vector<post> posts;

int main() {

    cin.tie(NULL);
    cout.tie(NULL);
    ios::sync_with_stdio(false);

    // writtingフォルダーを巡って分析
    for (auto& i : filesystem::directory_iterator("./original/writting")) {
        if (i.is_regular_file()) {
            ifstream f(i.path());
            if (f.is_open()) {
                analyze_file(f);
                f.close();
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
    for (auto& i : filesystem::directory_iterator("./post/writting")) {
        filesystem::remove(i.path());
    }

    // post/writting 新データ装入
    for (int i = 0; i < posts.size(); i++) {
        const string new_title = "./post/writting/" + posts[i].get_file_name() + ".html";
        string new_post = create_post(posts[i], tmplt);

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

    int writting_idx_start = new_blog.find("<!--writting_auto_add_start-->") + 29;
    int writting_idx_end = new_blog.find("<!--writting_auto_add_end-->");

    // 既存データ削除
    new_blog.erase(writting_idx_start + 24, writting_idx_end - writting_idx_start - 24);

    // データ更新をブログに反映
    string added_catalog;
    for (int i = 0; i < posts.size(); i++) {
        string title = posts[i].get_title();
        string file_name = posts[i].get_file_name();
        added_catalog += "<li><a href=\"#\" onclick=\"load_post('";
        added_catalog +=  file_name + "')\">" + title;
        added_catalog += "</a></li>\n";
    }
    new_blog.insert(writting_idx_start + 24, added_catalog);

    ofstream update("index.html", iostream::trunc);
    if (update.is_open()) {
        update << new_blog;
        update.close();
    }

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
    string tag = find_tag(str);
    string body = find_body(str);

    post mypost = { file_name, title, date, tag, body };
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
    tmplt.replace(tmplt.find("{{tags}}"), 8, mypost.get_tags());
    tmplt.replace(tmplt.find("{{body}}"), 8, mypost.get_body());
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