#include <catch.hpp>
#include <map>
#include <string>

#include <boost/variant/get.hpp>

#include "../../src/htmlparser/htmlparser.h"
#include "../../src/htmlparser/tokens/StartToken.h"
#include "../../src/htmlparser/tokens/DoctypeToken.h"

TEST_CASE("HTML tokenization")
{
    std::map<std::wstring, std::wstring> empty_map;
    SECTION("Manually creating tokens")
    {
        SECTION("Manually creating doctype token")
        {
            DoctypeToken doctype_token = DoctypeToken();
            CHECK_FALSE(doctype_token.quirks_required());
            CHECK_FALSE(doctype_token.is_name_set());
            CHECK_FALSE(doctype_token.is_public_identifier_set());
            CHECK_FALSE(doctype_token.is_system_identifier_set());
        }

        SECTION("Manually creating HTML root token")
        {
            StartToken html_token = StartToken();

            CHECK_FALSE(html_token.is_self_closing());
            CHECK(html_token.get_attributes() == empty_map);
            CHECK(html_token.get_tag_name() == L"");
        }
    }

    SECTION("Creating tokens with parser")
    {
        HTMLParser parser;

        SECTION("Creating doctype tokens with parser")
        {
            SECTION("Normal doctype token")
            {

                DoctypeToken token = 
                    boost::get<DoctypeToken>(parser.create_token_from_string(
                                L"<!DOCTYPE html>"));

                CHECK_FALSE(token.quirks_required());
                CHECK(token.is_name_set());
                CHECK(token.get_name() == L"html");
            }

            SECTION("Doctype name not set")
            {
                DoctypeToken token = 
                    boost::get<DoctypeToken>(parser.create_token_from_string(
                                L"<!DOCTYPE>"));

                CHECK(token.quirks_required());
                CHECK_FALSE(token.is_name_set());
            }

            SECTION("Correctly handles extra identifiers")
            {
                std::wstring long_doctype = L"<!DOCTYPE HTML PUBLIC "  
                    L"\"-//W3C//DTD HTML 4.01 Transitional//EN\" "
                    L"\"http://www.w3.org/TR/html4/loose.dtd\">";
                DoctypeToken token = 
                    boost::get<DoctypeToken>(parser.create_token_from_string(
                                long_doctype));

                CHECK(token.quirks_required());
                CHECK(token.get_name() == L"html");
                CHECK(token.is_name_set());
            }
        }

        SECTION("Creating start tokens with parser")
        {
            std::map<std::wstring, std::wstring> lang_map = {{L"lang", L"en"}};

            SECTION("Creating html root token with parser")
            {
                StartToken token = 
                    boost::get<StartToken>(parser.create_token_from_string(
                                L"<HtMl lang=\"en\">"));
                std::map<std::wstring, std::wstring> attributes_map = 
                    token.get_attributes();
                CHECK_FALSE(token.is_self_closing());
                CHECK(token.contains_attribute(L"lang"));
                CHECK(token.get_attribute_value(L"lang") == L"en");
                CHECK(token.get_tag_name() == L"html");
            }

            SECTION("Creating self-closing tag with parser")
            {
                StartToken token = boost::get<StartToken>
                    (parser.create_token_from_string(L"<br/>"));
                CHECK(token.is_self_closing());
                CHECK(token.get_tag_name() == L"br");
            }

            SECTION("Tag with multiple attributes")
            {
                StartToken token = 
                    boost::get<StartToken>(parser.create_token_from_string(
                        L"<img src=\"example.png\" width='10' height='20'>"));
                std::map<std::wstring, std::wstring> attributes_map = 
                    token.get_attributes();
                CHECK(token.get_tag_name() == L"img");
                CHECK_FALSE(token.is_self_closing());
                CHECK(token.contains_attribute(L"src"));
                CHECK(token.contains_attribute(L"width"));
                CHECK(token.contains_attribute(L"height"));
                CHECK(token.get_attribute_value(L"src") == L"example.png");
                CHECK(token.get_attribute_value(L"width") == L"10");
                CHECK(token.get_attribute_value(L"height") == L"20");
            }

            SECTION("Tag with repeated attributes")
            {
                StartToken token = 
                    boost::get<StartToken>(parser.create_token_from_string(
                                L"<html lang='en' lang='br'>"));
                CHECK(token.get_tag_name() == L"html");
                CHECK(token.contains_attribute(L"lang"));
                CHECK(token.get_attribute_value(L"lang") == L"en");
                CHECK_FALSE(token.get_attribute_value(L"lang") == L"br");
                CHECK_FALSE(token.is_self_closing());
            }

            SECTION("Capitalization in attribute name/value")
            {
                StartToken token = 
                    boost::get<StartToken>(parser.create_token_from_string(
                                L"<html lAnG='eN'>"));
                CHECK(token.get_tag_name() == L"html");
                CHECK(token.contains_attribute(L"lang"));
                CHECK(token.get_attribute_value(L"lang") == L"en");
                CHECK_FALSE(token.contains_attribute(L"lAnG"));
                CHECK_FALSE(token.is_self_closing());
            }

            SECTION("Self-closing tag with attributes")
            {
                StartToken token = 
                    boost::get<StartToken>(parser.create_token_from_string(
                                L"<area shape=\"circle\"/>"));
                CHECK(token.get_tag_name() == L"area");
                CHECK(token.is_self_closing());
                CHECK(token.contains_attribute(L"shape"));
                CHECK(token.get_attribute_value(L"shape") == L"circle");
            }
        }

        SECTION("Creating end tags with parser")
        {
            EndToken token = boost::get<EndToken>(
                    parser.create_token_from_string(L"</p>"));

            CHECK(token.get_tag_name() == L"p");
            CHECK_FALSE(token.is_self_closing());
        }
    }
}
