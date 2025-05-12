#include <utility>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>
#include <format>

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
string txtformat_to_htmlformat(string s, string file_name);
string erase_original_catalog(string new_blog);
string update_catalog(string s);
string update_recent_calaog(string new_blog);
void update_writting_catalog(string &s);
void update_tech_catalog(string &s);
void update_news_catalog(string &s);
bool sort_old(post a, post b);
bool sort_recent(post a, post b);
string which_folder(string const& tag);
void generate_sitemap();

// グローバル変数
vector<post> posts;
vector<pair<string, string>> tags_list = { 
    {"日常", "daily"},
    {"ニュース","news"},
    {"技術","tech"},
    {"作文","writting"}
};

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

    generate_sitemap();

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

    string new_tag = find_tag(str);

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
    tmplt.replace(tmplt.find("{{tags}}"), 8, mypost.get_tags());

    string body = txtformat_to_htmlformat(mypost.get_body(), mypost.get_file_name());
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

bool sort_old(post a, post b) {
    return stoi(a.get_date()) > stoi(b.get_date());
}

bool sort_recent(post a, post b) {
    return stoi(a.get_date()) < stoi(b.get_date());
}

string txtformat_to_htmlformat(string s, string file_name) {
    // \n -> <br>
    size_t pos = 0;
    while ((pos = s.find('\n', pos)) != string::npos) {
        s.replace(pos, 1, "<br>");
        pos += 4;
    }

    // {{img1}} -> <img src=....
    int i = 0;
    pos = 0;
    string target_erase = "<img src";
    while (1) {
        pos = s.find(target_erase, pos);
        if (pos != string::npos) {
            int erase_to = s.find(">", pos);
            s.erase(pos, erase_to - pos);
        } else break;
    }


    i = 0;
    pos = 0;
    while (1) {
        string target = "{{img" + to_string(i) + "}}";
        string img = format("<img src=\"../../pic/{0}/{1}.jpg\" alt=\"image load failed\">", file_name, i);
        if (s.find(target, pos) != string::npos) {
            pos = s.find(target, pos);
            cout << target << " -> " << img << "\n";
            s.replace(pos, target.length(), img);
        }
        else {
            break;
        }

        i++;
    }

    return s;
}

string erase_original_catalog(string new_blog) {
    for (int i = 0; i < tags_list.size(); i++) {
        string tag_ = tags_list[i].second;
        string start_txt = format("<!--{}_auto_add_start-->", tag_);
        string end_txt = format("<!--{}_auto_add_end-->", tag_);
        int writting_idx_start = new_blog.find(start_txt) + start_txt.length();
        int writting_idx_end = new_blog.find(end_txt);
        new_blog.erase(writting_idx_start, writting_idx_end - writting_idx_start);
    }
    return new_blog;
}

string update_catalog(string new_blog) {

    new_blog = erase_original_catalog(new_blog);

    for (int i = 0; i < posts.size(); i++) {
        string tag_ = posts[i].get_tags();
        string start_txt = format("<!--{}_auto_add_start-->", which_folder(tag_));
        string end_txt = format("<!--{}_auto_add_end-->", which_folder(tag_));

        int writting_idx_start = new_blog.find(start_txt) + start_txt.length();

        string title = posts[i].get_title();
        string file_name = posts[i].get_file_name();
        string writting_catalog = "";

        // 例："load_post('[25_04_19]WindowsCppEncoding', 'tech')">[25_04_19]WindowsC++のシステムコール＆エンコーディング問題</a></li>
        writting_catalog += "<li><a href=\"#\" onclick=\"load_post('";
        writting_catalog +=  file_name + "', '" + which_folder(posts[i].get_tags());
        writting_catalog += "')\">" + title;
        writting_catalog += "</a></li>\n";

        new_blog.insert(writting_idx_start, writting_catalog);
    }
    new_blog = update_recent_calaog(new_blog);
    return new_blog;
}

string update_recent_calaog(string new_blog) {
    int posts_size = posts.size();
    const int max_idx = min(3, posts_size);
    string start_txt = "<!--recent_post_start-->";
    string end_txt = "<!--recent_post_end-->";
    string catalog = "";
    int idx_start = new_blog.find(start_txt) + start_txt.length();
    int idx_end = new_blog.find(end_txt);
    new_blog.erase(idx_start, idx_end - idx_start);


    for (int i = 0; i < max_idx; i++) {
        catalog += format("<a href=\"#\" onclick=\"load_post('{0}', '{1}')\">{2}</a>\n<br>\n",
            posts[i].get_file_name(),
            which_folder(posts[i].get_tags()),
            posts[i].get_title()
        );
        new_blog.insert(idx_start, catalog);
        catalog = "";
    }
    return new_blog;
}

string which_folder(string const& tag) {
    for (int i = 0; i < tags_list.size(); i++) {
        if (tags_list[i].first == tag) return tags_list[i].second;
    }
    return "writting";
}

void generate_sitemap() {
    ofstream sitemap("sitemap.xml", ios::trunc);
    if (!sitemap.is_open()) {
        return;
    }

    sitemap << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
    sitemap << "<urlset xmlns=\"http://www.sitemaps.org/schemas/sitemap/0.9\">\n";

    sitemap << "<url>\n";
    sitemap << "<loc>https://goranipie.github.io/</loc>\n";
    sitemap << "</url>\n";

    for (auto p : posts) {
        string url = format("https://goranipie.github.io/post/{0}/{1}.html",
            which_folder(p.get_tags()),
            p.get_file_name()
        );

        string date = p.get_date();  // YYYYMMDD
        date.insert(4, "-");
        date.insert(7, "-");  // YYYY-MM-DD

        sitemap << "<url>\n";
        sitemap << "<loc>" << url << "</loc>\n";
        sitemap << "<lastmod>" << date << "</lastmod>\n";
        sitemap << "</url>\n";
    }

    sitemap << "</urlset>\n";
    sitemap.close();

}
