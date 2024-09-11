#pragma once

#include <string>

using std::string, std::string_view;

/*
 * Декодирует основные HTML-мнемоники:
 * - &lt - <
 * - &gt - >
 * - &amp - &
 * - &pos - '
 * - &quot - "
 *
 * Мнемоника может быть записана целиком либо строчными, либо заглавными буквами:
 * - &lt и &LT декодируются как <
 * - &Lt и &lT не мнемоники
 *
 * После мнемоники может стоять опциональный символ ;
 * - M&amp;M&APOSs декодируется в M&M's
 * - &amp;lt; декодируется в &lt;
 */
std::string HtmlDecode(std::string_view str);