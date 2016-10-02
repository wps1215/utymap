#include "config.hpp"
#include "mapcss/StyleSheet.hpp"
#include "mapcss/MapCssParser.cpp"
#include "utils/CoreUtils.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap::mapcss;
using namespace utymap::meshing;

namespace {
    typedef std::string::const_iterator StringIterator;

    struct MapCss_MapCssParserFixture {
        MapCss_MapCssParserFixture() :
            stylesheet(),
            styleSheetGrammar("", stylesheet)
        {
        }

        // parsed data
        StyleSheet stylesheet;
        Condition condition;
        Zoom zoom;
        Selector selector;
        Declaration declaration;
        Rule rule;
        // grammars
        CommentSkipper<StringIterator> skipper;
        StyleSheetGrammar<StringIterator> styleSheetGrammar;
        ConditionGrammar<StringIterator> conditionGrammar;
        ZoomGrammar<StringIterator> zoomGrammar;
        SelectorGrammar<StringIterator> selectorGrammar;
        DeclarationGrammar<StringIterator> declarationGrammar;
        RuleGrammar<StringIterator> ruleGrammar;
    };
}

BOOST_FIXTURE_TEST_SUITE(MapCss_MapCssParser, MapCss_MapCssParserFixture)

/* Comment */
BOOST_AUTO_TEST_CASE(GivenCppComment_WhenParse_ThenDoesNotBreak)
{
    std::string str = "/* Some \nncomment */\n way|z1-15[highway] {key:value;}";

    bool success = phrase_parse(str.cbegin(), str.cend(), styleSheetGrammar, skipper, stylesheet);

    BOOST_CHECK(success);
    BOOST_CHECK_EQUAL(stylesheet.rules.size(), 1);
}

BOOST_AUTO_TEST_CASE(GivenHtmlComment_WhenParse_ThenDoesNotBreak)
{
    std::string str = "<!--Some \ncomment-->\n way|z1-15[highway] {key:value;}";

    bool success = phrase_parse(str.cbegin(), str.cend(), styleSheetGrammar, skipper, stylesheet);

    BOOST_CHECK(success);
    BOOST_CHECK_EQUAL(stylesheet.rules.size(), 1);
}

/* Condition */
BOOST_AUTO_TEST_CASE(GivenExistenceCondition_WhenParse_ThenOnlyKeyIsSet)
{
    std::string str = "[highway]";

    bool success = phrase_parse(str.cbegin(), str.cend(), conditionGrammar, skipper, condition);

    BOOST_CHECK(success);
    BOOST_CHECK_EQUAL(condition.key, "highway");
    BOOST_CHECK(condition.operation.empty());
    BOOST_CHECK(condition.value.empty());
}

BOOST_AUTO_TEST_CASE(GivenEqualCondition_WhenParse_ThenKeyOpValueAreSet)
{
    std::string str = "[highway=primary]";

    bool success = phrase_parse(str.cbegin(), str.cend(), conditionGrammar, skipper, condition);

    BOOST_CHECK(success);
    BOOST_CHECK_EQUAL(condition.key, "highway");
    BOOST_CHECK_EQUAL(condition.operation, "=");
    BOOST_CHECK_EQUAL(condition.value, "primary");
}

BOOST_AUTO_TEST_CASE(GivenNegativeCondition_WhenParse_ThenKeyOpValueAreSet)
{
    std::string str = "[highway!=primary]";

    bool success = phrase_parse(str.cbegin(), str.cend(), conditionGrammar, skipper, condition);

    BOOST_CHECK(success);
    BOOST_CHECK_EQUAL(condition.key, "highway");
    BOOST_CHECK_EQUAL(condition.operation, "!=");
    BOOST_CHECK_EQUAL(condition.value, "primary");
}

/* Zoom */
BOOST_AUTO_TEST_CASE(GivenOneZoom_WhenParse_ThenStartAndEndAreSet)
{
    std::string str = "|z1";

    bool success = phrase_parse(str.cbegin(), str.cend(), zoomGrammar, skipper, zoom);

    BOOST_CHECK(success);
    BOOST_CHECK_EQUAL(zoom.start, 1);
    BOOST_CHECK_EQUAL(zoom.end, 1);
}

BOOST_AUTO_TEST_CASE(GivenZoomRange_WhenParse_ThenStartAndEndAreSet)
{
    std::string str = "|z12-21";

    bool success = phrase_parse(str.cbegin(), str.cend(), zoomGrammar, skipper, zoom);

    BOOST_CHECK(success);
    BOOST_CHECK_EQUAL(zoom.start, 12);
    BOOST_CHECK_EQUAL(zoom.end, 21);
}

/* Selector */
BOOST_AUTO_TEST_CASE(GivenSingleSelector_WhenParse_ThenNameAndConditionsAreSet)
{
    std::string str = "way|z1[highway]";

    bool success = phrase_parse(str.cbegin(), str.cend(), selectorGrammar, skipper, selector);

    BOOST_CHECK(success);
    BOOST_CHECK_EQUAL(selector.names.size(), 1);
    BOOST_CHECK_EQUAL(selector.names[0], "way");
    BOOST_CHECK_EQUAL(selector.conditions.size(), 1);
    BOOST_CHECK_EQUAL(selector.conditions[0].key, "highway");
}

BOOST_AUTO_TEST_CASE(GivenTwoSelectors_WhenParse_ThenNameAndConditionsAreSet)
{
    std::string str = "way|z1[highway][landuse]";

    bool success = phrase_parse(str.cbegin(), str.cend(), selectorGrammar, skipper, selector);

    BOOST_CHECK(success);
    BOOST_CHECK_EQUAL(selector.names.size(), 1);
    BOOST_CHECK_EQUAL(selector.names[0], "way");
    BOOST_CHECK_EQUAL(selector.conditions.size(), 2);
    BOOST_CHECK_EQUAL(selector.conditions[0].key, "highway");
    BOOST_CHECK_EQUAL(selector.conditions[1].key, "landuse");
}

BOOST_AUTO_TEST_CASE(GivenTwoSelectorNames_WhenParse_ThenNamesAndConditionsAreSet)
{
    std::string str = "area,relation|z1[building][part]";

    bool success = phrase_parse(str.cbegin(), str.cend(), selectorGrammar, skipper, selector);

    BOOST_CHECK(success);
    BOOST_CHECK_EQUAL(selector.names.size(), 2);
    BOOST_CHECK_EQUAL(selector.names[0], "area");
    BOOST_CHECK_EQUAL(selector.names[1], "relation");
    BOOST_CHECK_EQUAL(selector.conditions.size(), 2);
    BOOST_CHECK_EQUAL(selector.conditions[0].key, "building");
    BOOST_CHECK_EQUAL(selector.conditions[1].key, "part");
}

/* Declaration */
BOOST_AUTO_TEST_CASE(GivenSingleDeclaraion_WhenParse_ThenKeyValueAreSet)
{
    std::string str = "key1:value1;";

    bool success = phrase_parse(str.cbegin(), str.cend(), declarationGrammar, skipper, declaration);

    BOOST_CHECK(success);
    BOOST_CHECK_EQUAL(declaration.key, "key1");
    BOOST_CHECK_EQUAL(declaration.value, "value1");
}

BOOST_AUTO_TEST_CASE(GivenGradientDeclaraion_WhenParse_ThenGradientValueIsCorrect)
{
    std::string str = "color:gradient(#dcdcdc 0%, #c0c0c0 10%, #a9a9a9 50%, #808080);";

    bool success = phrase_parse(str.cbegin(), str.cend(), declarationGrammar, skipper, declaration);

    BOOST_CHECK(success);
    BOOST_CHECK_EQUAL(declaration.key, "color");
    BOOST_CHECK_EQUAL(declaration.value, "gradient(#dcdcdc 0%, #c0c0c0 10%, #a9a9a9 50%, #808080)");
}

/* Rule */
BOOST_AUTO_TEST_CASE(GivenSimpleRule_WhenParse_ThenSelectorAndDeclarationAreSet)
{
    std::string str = "way|z1[highway] {key1:value1;}";

    bool success = phrase_parse(str.cbegin(), str.cend(), ruleGrammar, skipper, rule);

    BOOST_CHECK(success);
    BOOST_CHECK_EQUAL(rule.selectors.size(), 1);
    BOOST_CHECK_EQUAL(rule.declarations.size(), 1);
    BOOST_CHECK_EQUAL(rule.selectors[0].conditions[0].key , "highway");
    BOOST_CHECK_EQUAL(rule.declarations[0].key, "key1");
    BOOST_CHECK_EQUAL(rule.declarations[0].value,"value1");
}

BOOST_AUTO_TEST_CASE(GivenComplexRule_WhenParse_ThenSelectorAndDeclarationAreSet)
{
    std::string str = "way|z1[highway],area|z2[landuse] { key1:value1; key2:value2; }";

    bool success = phrase_parse(str.cbegin(), str.cend(), ruleGrammar, skipper, rule);

    BOOST_CHECK(success);
    BOOST_CHECK_EQUAL(rule.selectors.size(), 2);
    BOOST_CHECK_EQUAL(rule.declarations.size(), 2);
    BOOST_CHECK_EQUAL(rule.selectors[0].names.size(), 1);
    BOOST_CHECK_EQUAL(rule.selectors[0].names[0], "way");
    BOOST_CHECK_EQUAL(rule.selectors[1].names.size(), 1);
    BOOST_CHECK_EQUAL(rule.selectors[1].names[0], "area");
    BOOST_CHECK_EQUAL(rule.selectors[0].conditions[0].key, "highway");
    BOOST_CHECK_EQUAL(rule.selectors[1].conditions[0].key, "landuse");
    BOOST_CHECK_EQUAL(rule.declarations[0].key,"key1");
    BOOST_CHECK_EQUAL(rule.declarations[0].value, "value1");
    BOOST_CHECK_EQUAL(rule.declarations[1].key, "key2");
    BOOST_CHECK_EQUAL(rule.declarations[1].value, "value2");
}

/* StyleSheet */
BOOST_AUTO_TEST_CASE(GivenFourRulesOnDifferentLines_WhenParse_ThenHasFourRules)
{
    std::string str =
        "way|z1[highway]  { key1:value1; }\n"
        "area|z2[landuse] { key2:value2; }\n"
        "node|z3[amenity] { key3:value3; }\n"
        "canvas|z3        { key4:value4; }";

    phrase_parse(str.cbegin(), str.cend(), styleSheetGrammar, skipper, stylesheet);

    BOOST_CHECK_EQUAL( stylesheet.rules.size(), 4) ;
}

BOOST_AUTO_TEST_CASE(GivenImport_WhenParse_ThenStyleSheetIsImported)
{
    std::string str = "@import url(\"import.mapcss\");";
    ImportGrammar<StringIterator> importGrammar(TEST_MAPCSS_PATH, stylesheet);

    bool success = phrase_parse(str.cbegin(), str.cend(), importGrammar, skipper);

    BOOST_CHECK(success);
}

BOOST_AUTO_TEST_CASE(GivenSimpleStyleSheet_WhenParserParse_ThenNoErrorsAndHasValidStyleSheet)
{
    std::string str = "way|z1[highway]  { key1:value1; }\n";
    MapCssParser parser;

    StyleSheet stylesheet = parser.parse(str);

    BOOST_CHECK_EQUAL(stylesheet.rules.size(), 1);
}

BOOST_AUTO_TEST_CASE(GivenCanvasRule_WhenParse_ThenProcessValidStyleSheet)
{
    std::string str = "canvas|z1 { key1:value1; }\n";
    MapCssParser parser;

    StyleSheet stylesheet = parser.parse(str);

    BOOST_CHECK_EQUAL(stylesheet.rules.size(), 1);
}


BOOST_AUTO_TEST_CASE(GivenSimpleRuleWithZoomRange_WhenParse_ThenReturnCorrectZoomStartAndEnd)
{
    std::string str = "way|z1-12[highway]{key1:value1;}";
    MapCssParser parser;

    StyleSheet stylesheet = parser.parse(str);

    Selector selector = stylesheet.rules[0].selectors[0];
    BOOST_CHECK_EQUAL(selector.zoom.start, 1);
    BOOST_CHECK_EQUAL(selector.zoom.end, 12);
}

BOOST_AUTO_TEST_CASE(GivenRuleWithGradient_WhenParse_ThenReturnCorrectGradientValue)
{
    std::string str = "way|z1-12[highway]{color:gradient(#dcdcdc 0%, #c0c0c0 10%, #a9a9a9 50%, #808080);}";
    MapCssParser parser;

    StyleSheet stylesheet = parser.parse(str);

    BOOST_CHECK_EQUAL(stylesheet.rules.size(), 1);
    BOOST_CHECK_EQUAL(stylesheet.rules[0].declarations[0].value, "gradient(#dcdcdc 0%, #c0c0c0 10%, #a9a9a9 50%, #808080)");
}

BOOST_AUTO_TEST_CASE(GivenSimpleRule_WhenToString_ThenReturnCorrectRepresentation)
{
    std::string str = "way|z1-12[highway]{key1:value1;}";
    MapCssParser parser;

    StyleSheet stylesheet = parser.parse(str);

    BOOST_CHECK_EQUAL(stylesheet.rules.size(), 1);
    BOOST_CHECK_EQUAL(utymap::utils::toString(stylesheet.rules[0]), str);
}

BOOST_AUTO_TEST_CASE(GivenImportFile_WhenParse_ThenAllRulesAreMerged)
{
    std::ifstream styleFile(TEST_MAPCSS_PATH "import.mapcss");
    MapCssParser parser(TEST_MAPCSS_PATH);

    StyleSheet stylesheet = parser.parse(styleFile);

    BOOST_CHECK_EQUAL(stylesheet.rules.size(), 5);
}

BOOST_AUTO_TEST_CASE(GivenTwoTextureRegions_WhenParse_ThenHasTwoRegions)
{
    std::ifstream styleFile(TEST_MAPCSS_PATH "import.mapcss");
    MapCssParser parser(TEST_MAPCSS_PATH);

    StyleSheet stylesheet = parser.parse(styleFile);

    BOOST_CHECK_EQUAL(stylesheet.textures.size(), 2);
}

BOOST_AUTO_TEST_SUITE_END()
