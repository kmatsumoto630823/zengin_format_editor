#include "FBDataParser.h"

#include <fstream>
#include <algorithm>
#include <wx/log.h>

constexpr std::size_t FB_WIDTH = 120;
constexpr std::size_t FB_HEADER_KB  = 1;
constexpr std::size_t FB_DATA_KB    = 2;
constexpr std::size_t FB_TRAILER_KB = 8;
constexpr std::size_t FB_END_KB     = 9;

constexpr char char_num []  = "0123456789";
constexpr char pad_num []   = "L0";

constexpr char char_kana []  = " ()-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜﾝﾞﾟ";
constexpr char pad_kana []   = "R ";

constexpr char char_dummy []  = " ";
constexpr char pad_dummy []   = "R ";

const FBAttrs FBDataParser::fb_header_attrs_sohfuri =
{
    { 0, "データ区分",     0,  1, "1",        "L1",      nullptr,   "文字種（桁）：Ｎ（１）\r\n必須項目\r\n「１」：ヘッダーレコード"},
    { 1, "種別コード",     1,  2, char_num,   pad_num,   nullptr,   "文字種（桁）：Ｎ（２）\r\n必須項目\r\n「２１」：総合振込"},
    { 2, "コード区分",     3,  1, char_num,   pad_num,   nullptr,   "文字種（桁）：Ｎ（１）\r\n必須項目\r\n「０」：ＳＪＩＳ\r\n「１」：ＥＢＣＤＩＣ"},
    { 3, "依頼人コード",   4, 10, char_num,   pad_num,   nullptr,   "文字種（桁）：Ｎ（１０）\r\n必須項目\r\n振込依頼人識別のため銀行が採番したコード"},
    { 4, "依頼人名",      14, 40, char_kana,  pad_kana,  nullptr,   "文字種（桁）：Ｃ（４０）\r\n必須項目"},
    { 5, "取組日",        54,  4, char_num,   pad_num,   nullptr,   "文字種（桁）：Ｎ（４）\r\n必須項目\r\nＭＭＤＤ（ＭＭ月ＤＤ日）"},
    { 6, "仕向銀行番号",  58,  4, char_num,   pad_num,   nullptr,   "文字種（桁）：Ｎ（４）\r\n必須項目\r\n統一金融機関番号"},
    { 7, "仕向銀行名",    62, 15, char_kana,  pad_kana,  nullptr,   "文字種（桁）：Ｃ（１５）\r\n任意項目"},
    { 8, "仕向支店番号",  77,  3, char_num,   pad_num,   nullptr,   "文字種（桁）：Ｎ（３）\r\n必須項目\r\n統一店番号"},
    { 9, "仕向支店名",    80, 15, char_kana,  pad_kana,  nullptr,   "文字種（桁）：Ｃ（１５）\r\n任意項目"},
    {10, "預金種目",      95,  1, char_num,   pad_num,   " ",       "文字種（桁）：Ｎ（１）\r\n任意項目\r\n「１」：普通預金\r\n「２」：当座預金\r\n「９」：その他"},
    {11, "口座番号",      96,  7, char_num,   pad_num,   "       ", "文字種（桁）：Ｎ（７）\r\n任意項目"},
    {12, "ダミー",       103, 17, char_dummy, pad_dummy, nullptr,   "文字種（桁）：Ｃ（１７）\r\n必須項目\r\nスペースとする"}
};

const FBAttrs FBDataParser::fb_data_attrs_sohfuri =
{
    { 0, "データ区分",      0,  1, "2",        "L2",      nullptr, "文字種（桁）：Ｎ（１）\r\n必須項目\r\n「２」：データーレコード"},
    { 1, "銀行番号",        1,  4, char_num,   pad_num,   nullptr, "文字種（桁）：Ｎ（４）\r\n必須項目\r\n統一金融機関番号"},
    { 2, "銀行名",          5, 15, char_kana,  pad_kana,  nullptr, "文字種（桁）：Ｃ（１５）\r\n任意項目"},
    { 3, "支店番号",       20,  3, char_num,   pad_num,   nullptr, "文字種（桁）：Ｎ（３）\r\n必須項目\r\n統一店番号"},
    { 4, "支店名",         23, 15, char_kana,  pad_kana,  nullptr, "文字種（桁）：Ｃ（１５）\r\n任意項目"},
    { 5, "手形交換所番号", 38,  4, char_num,   pad_num,   "    ",  "文字種（桁）：Ｎ（４）\r\n 任意項目\r\n統一手形交換所番号"},
    { 6, "預金種目",       42,  1, char_num,   pad_num,   nullptr, "文字種（桁）：Ｎ（１）\r\n必須項目\r\n「１」：普通預金\r\n「２」：当座預金\r\n「４」：貯蓄預金\r\n「９」：その他"},
    { 7, "口座番号",       43,  7, char_num,   pad_num,   nullptr, "文字種（桁）：Ｎ（７）\r\n必須項目"},
    { 8, "受取人名",       50, 30, char_kana,  pad_kana,  nullptr, "文字種（桁）：Ｃ（３０）\r\n必須項目"},
    { 9, "振込金額",       80, 10, char_num,   pad_num,   nullptr, "文字種（桁）：Ｎ（１０）\r\n必須項目"},
    {10, "新規コード",     90,  1, char_num,   pad_num,   " ",     "文字種（桁）：Ｎ（１）\r\n必須項目\r\n「１」：第１回振込分\r\n「２」：変更分（銀行・支店、預金種目・口座番号）\r\n「０」：その他"},
    {11, "顧客コード1",    91, 10, char_kana,  pad_kana,  nullptr, "文字種（桁）：Ｎ（１0）\r\n任意項目\r\n依頼人が定めた受取人識別のための顧客コード"},
    {12, "顧客コード2",   101, 10, char_kana,  pad_kana,  nullptr, "文字種（桁）：Ｎ（１0）\r\n任意項目\r\n依頼人が定めた受取人識別のための顧客コード"},
    {13, "振込区分",      111,  1, char_num,   pad_num,   " ",     "文字種（桁）：Ｎ（１）\r\n任意項目\r\n「７」：テレ振込\r\n「８」：文書振込"},
    {14, "識別表\示",     112,  1, char_kana,  pad_kana,  nullptr, "文字種（桁）：Ｃ（１）\r\n任意項目\r\n「Ｙ」またはスペース\r\n「Ｙ」の場合、「顧客コード１」「顧客コード２」は「EDI情報」となる"},
    {15, "ダミー",        113,  7, char_dummy, pad_dummy, nullptr, "文字種（桁）：Ｃ（７）\r\n必須項目\r\nスペースとする"}
};

const FBAttrs FBDataParser::fb_trailer_attrs_sohfuri =
{
    { 0, "データ区分",  0,   1, "8",        "L8",      nullptr, "文字種（桁）：Ｎ（１）\r\n必須項目\r\n「８」：トレーラーレコード"},
    { 1, "合計件数",    1,   6, char_num,   pad_num,   nullptr, "文字種（桁）：Ｎ（６）\r\n必須項目"},
    { 2, "合計金額",    7,  12, char_num,   pad_num,   nullptr, "文字種（桁）：Ｎ（１２）\r\n必須項目"},
    { 3, "ダミー",     19, 101, char_dummy, pad_dummy, nullptr, "文字種（桁）：Ｃ（１０１）\r\n必須項目\r\nスペースとする"}
};

const FBAttrs FBDataParser::fb_end_attrs_sohfuri =
{
    { 0, "データ区分", 0,   1, "9", "L9", nullptr, "文字種（桁）：Ｎ（１）\r\n必須項目\r\n「９」：エンドレコード"},
    { 1, "ダミー",     1, 119, char_dummy, pad_dummy, nullptr, "文字種（桁）：Ｃ（１１９）\r\n必須項目\r\nスペースとする"}
};

FBDataParser::FBDataParser() : m_null_str("")
{
    m_newline_code = "\r\n";

    for(const auto &attr : FBDataParser::fb_header_attrs){
        if(attr.initial_value != nullptr){
            if(attr.length != std::string_view(attr.initial_value).length()){
                wxLogMessage("length != std::string_view(initial_value).length()");
            }
        }
    }

    for(const auto &attr : FBDataParser::fb_data_attrs){
        if(attr.initial_value != nullptr){
            if(attr.length != std::string_view(attr.initial_value).length()){
                wxLogMessage("length != std::string_view(initial_value).length()");
            }
        }
    }

    for(const auto &attr : FBDataParser::fb_trailer_attrs){
        if(attr.initial_value != nullptr){
            if(attr.length != std::string_view(attr.initial_value).length()){
                wxLogMessage("length != std::string_view(initial_value).length()");
            }
        }
    }

    for(const auto &attr : FBDataParser::fb_end_attrs){
        if(attr.initial_value != nullptr){
            if(attr.length != std::string_view(attr.initial_value).length()){
                wxLogMessage("length != std::string_view(initial_value).length()");
            }
        }
    }

    fb_header_attrs = fb_header_attrs_sohfuri;
    fb_data_attrs = fb_data_attrs_sohfuri;
    fb_trailer_attrs = fb_trailer_attrs_sohfuri;
    fb_end_attrs = fb_end_attrs_sohfuri;
}

FBDataParser::~FBDataParser()
{
}

bool FBDataParser::open_fb_file(const std::string &path)
{
    std::ifstream fb_ifs(path, std::ios::binary);

    if(!fb_ifs)
    {
        wxLogMessage("fb_ifs : false");
        return false;
    }

    fb_ifs.seekg(0, std::ios_base::end);
    auto file_size = fb_ifs.tellg();
    if(file_size > 104857600){
        wxLogMessage("file_size > 104857600");
        return false;
    }

    fb_ifs.seekg(0, std::ios_base::beg);

    fb_header_lines.clear();
    fb_data_lines.clear();
    fb_trailer_lines.clear();
    fb_end_lines.clear();


    std::string fb_str;
    fb_str.reserve(file_size);
    auto fb_ifs_it = std::istreambuf_iterator<char>(fb_ifs);
    auto fb_ifs_last = std::istreambuf_iterator<char>();
    fb_str.assign(fb_ifs_it, fb_ifs_last);

    fb_str.erase(std::remove(fb_str.begin(), fb_str.end(), '\r'), fb_str.end());
    fb_str.erase(std::remove(fb_str.begin(), fb_str.end(), '\n'), fb_str.end());


    if(fb_str.size() % FB_WIDTH != 0)
    {
        wxLogMessage("fb_str.size() mod FB_WIDTH != 0");
        return false;
    }


    int record_kb = 0;
    for(int i = 0; i < fb_str.size(); i += FB_WIDTH)
    {       
        std::string_view fb_str_line(fb_str.data() + i, FB_WIDTH);

        if(record_kb > fb_str_line.at(0) - '0')
        {
            wxLogMessage("record_type > fb_str_line_view.at(0) - '0'");
            return false;
        }

        record_kb = fb_str_line.at(0) - '0';

        FBAttrs *fb_attrs_ref;
        FBLines *fb_lines_ref;

        switch(record_kb)
        {
            case FB_HEADER_KB:
                fb_attrs_ref = &fb_header_attrs;
                fb_lines_ref = &fb_header_lines;
                break;

            case FB_DATA_KB:
                fb_attrs_ref = &fb_data_attrs;
                fb_lines_ref = &fb_data_lines;
                break;
            
            case FB_TRAILER_KB:
                fb_attrs_ref = &fb_trailer_attrs;
                fb_lines_ref = &fb_trailer_lines;
                break;

            case FB_END_KB:
                fb_attrs_ref = &fb_end_attrs;
                fb_lines_ref = &fb_end_lines;
                break;

            default:
                wxLogMessage("record_kb : undefined");
                return false;
        }

        for(const auto &attr : *fb_attrs_ref)
        {
            auto fb_str_sub = fb_str_line.substr(attr.offset, attr.length);

            if(fb_str_sub.find_first_not_of(attr.char_includes) != std::string_view::npos)
            {
                if(attr.initial_value == nullptr || fb_str_sub != std::string_view(attr.initial_value)){
                    wxLogMessage("fb_str_sub.find_first_not_of(char_includes) != std::string_view::npos");
                    return false;
                }
            }
        }

        fb_lines_ref->emplace_back(fb_str_line);
    }

    return true;
}

bool FBDataParser::save_fb_file(const std::string &path)
{
    std::ofstream fb_ofs(path, std::ios::binary);
    if(!fb_ofs)
    {
        wxLogMessage("fb_ofs : false");
        return false;
    }
    
    for(const auto & line : fb_header_lines )fb_ofs << line << m_newline_code;
    for(const auto & line : fb_data_lines   )fb_ofs << line << m_newline_code;
    for(const auto & line : fb_trailer_lines)fb_ofs << line << m_newline_code;
    for(const auto & line : fb_end_lines    )fb_ofs << line << m_newline_code;

    return true;
};

bool FBDataParser::set_fb_newline_code(std::string_view newline_code)
{
    m_newline_code = newline_code;
    return true;
}

std::string_view FBDataParser::get_fb_newline_code()
{
    return m_newline_code;
}


std::size_t FBDataParser::get_fb_row_size(FBLines& fb_lines, FBAttrs& fb_attrs)
{
    return fb_lines.size();
}

bool FBDataParser::assign_fb_line(std::size_t num, FBLines& fb_lines, FBAttrs& fb_attrs)
{
    fb_lines.assign(num, std::string(FB_WIDTH, ' '));
    return true;
}


std::string_view FBDataParser::get_fb_value(std::size_t row, std::size_t col, FBLines& fb_lines, FBAttrs& fb_attrs)
{
    if(row >= fb_lines.size() || col >= fb_attrs.size())
    {
        wxLogMessage("row >= fb_lines.size() || col >= fb_attr.size()");
        return m_null_str;
    }
    
    const auto &attr = fb_attrs.at(col);

    std::string_view line = fb_lines.at(row);
    auto value = line.substr(attr.offset, attr.length);
    
    return value;
 
}

bool FBDataParser::set_fb_value(std::size_t row, std::size_t col, std::string_view value, FBLines& fb_lines, FBAttrs& fb_attrs)
{
    if(row >= fb_lines.size() || col >= fb_attrs.size())
    {
        wxLogMessage("row >= fb_lines.size() || col >= fb_attr.size()");
        return false;
    }

    const auto &attr = fb_attrs.at(col);
    if(value.size() != attr.length)
    {
        wxLogMessage("value.size() != length");
        return false;
    }


    if(value.find_first_not_of(attr.char_includes) != std::string_view::npos)
    {
        if(attr.initial_value == nullptr || value != std::string_view(attr.initial_value))
        {
            wxLogMessage("value.find_first_not_of(char_includes) != std::string_view::npos");
            return false;
        }
    }

    auto &line = fb_lines.at(row);
    std::copy(value.begin(), value.end(), line.begin() + attr.offset);

    return true;
}