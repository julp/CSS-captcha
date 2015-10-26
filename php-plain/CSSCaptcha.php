<?php
interface CSSCaptchaStoreInterface
{
    public function get($key);
    public function set($key, $data);
    public function remove($key);
}

class CSSCaptchaSessionStore implements CSSCaptchaStoreInterface
{
    private static function checkActiveSession()
    {
        if (function_exists('session_status') && PHP_SESSION_ACTIVE != session_status()) {
            throw new Exception('CSSCaptcha implies an active session');
        }
    }

    public function get($key)
    {
        self::checkActiveSession();

        return $_SESSION[$key];
    }

    public function set($key, $data)
    {
        self::checkActiveSession();
        $_SESSION[$key] = $data;

        return TRUE;
    }

    public function remove($key)
    {
        self::checkActiveSession();
        unset($_SESSION[$key]);

        return TRUE;
    }
}

class CSSCaptcha {

    const RENDER_CSS = 0x01;
    const RENDER_HTML = 0x10;

    const NEVER = __LINE__;
    const ALWAYS = __LINE__;
    const RANDOM = __LINE__;

    const COLOR_NONE = 0;
    const COLOR_RED = __LINE__;
    const COLOR_BLUE = __LINE__;
    const COLOR_GREEN = __LINE__;
    const COLOR_DARK = __LINE__;
    const COLOR_LIGHT = __LINE__;

    const UNICODE_1_1_0 = 1;
    const UNICODE_1_1_5 = 2;
    const UNICODE_2_0_0 = 3;
    const UNICODE_2_1_0 = 4;
    const UNICODE_2_1_2 = 5;
    const UNICODE_2_1_5 = 6;
    const UNICODE_2_1_8 = 7;
    const UNICODE_2_1_9 = 8;
    const UNICODE_3_0_0 = 9;
    const UNICODE_3_0_1 = 10;
    const UNICODE_3_1_0 = 11;
    const UNICODE_3_1_1 = 12;
    const UNICODE_3_2_0 = 13;
    const UNICODE_4_0_0 = 14;
    const UNICODE_4_0_1 = 15;
    const UNICODE_4_1_0 = 16;
    const UNICODE_5_0_0 = 17;
    const UNICODE_5_1_0 = 18;
    const UNICODE_5_2_0 = 19;
    const UNICODE_6_0_0 = 20;

    const UNICODE_FIRST = self::UNICODE_1_1_0;
    const UNICODE_LAST = self::UNICODE_6_0_0;

    const ATTR_ALPHABET = __LINE__;
    const ATTR_REVERSED = __LINE__;
    const ATTR_NOISE_LENGTH = __LINE__;
    const ATTR_SESSION_PREFIX = __LINE__;
    const ATTR_HTML_WRAPPER_ID = __LINE__;
    const ATTR_HTML_LETTER_TAG = __LINE__;
    const ATTR_HTML_WRAPPER_TAG = __LINE__;
    const ATTR_UNICODE_VERSION = __LINE__;
    const ATTR_CHALLENGE_LENGTH = __LINE__;
    const ATTR_FAKE_CHARACTERS_COLOR = __LINE__;
    const ATTR_FAKE_CHARACTERS_STYLE = __LINE__;
    const ATTR_FAKE_CHARACTERS_LENGTH = __LINE__;
    const ATTR_SKIP_UNICODE_FOR_CHALLENGE = __LINE__;
    const ATTR_SIGNIFICANT_CHARACTERS_COLOR = __LINE__;
    const ATTR_SIGNIFICANT_CHARACTERS_STYLE = __LINE__;

    protected static $_table = array(
        0x2070, 0x2080, 0x24EA, 0xFF10, 0x01D7CE, 0x01D7D8, 0x01D7E2, 0x01D7EC, 0x01D7F6, 0x00B9, 0x2081, 0x2460, 0xFF11, 0x01D7CF, 0x01D7D9, 0x01D7E3, 0x01D7ED, 0x01D7F7, 0x00B2, 0x2082,
        0x2461, 0xFF12, 0x01D7D0, 0x01D7DA, 0x01D7E4, 0x01D7EE, 0x01D7F8, 0x00B3, 0x2083, 0x2462, 0xFF13, 0x01D7D1, 0x01D7DB, 0x01D7E5, 0x01D7EF, 0x01D7F9, 0x2074, 0x2084, 0x2463, 0xFF14,
        0x01D7D2, 0x01D7DC, 0x01D7E6, 0x01D7F0, 0x01D7FA, 0x2075, 0x2085, 0x2464, 0xFF15, 0x01D7D3, 0x01D7DD, 0x01D7E7, 0x01D7F1, 0x01D7FB, 0x2076, 0x2086, 0x2465, 0xFF16, 0x01D7D4, 0x01D7DE,
        0x01D7E8, 0x01D7F2, 0x01D7FC, 0x2077, 0x2087, 0x2466, 0xFF17, 0x01D7D5, 0x01D7DF, 0x01D7E9, 0x01D7F3, 0x01D7FD, 0x2078, 0x2088, 0x2467, 0xFF18, 0x01D7D6, 0x01D7E0, 0x01D7EA, 0x01D7F4,
        0x01D7FE, 0x2079, 0x2089, 0x2468, 0xFF19, 0x01D7D7, 0x01D7E1, 0x01D7EB, 0x01D7F5, 0x01D7FF, 0x00AA, 0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00E0, 0x00E1, 0x00E2, 0x00E3,
        0x00E4, 0x00E5, 0x0100, 0x0101, 0x0102, 0x0103, 0x0104, 0x0105, 0x01CD, 0x01CE, 0x01DE, 0x01DF, 0x01E0, 0x01E1, 0x01FA, 0x01FB, 0x0200, 0x0201, 0x0202, 0x0203, 0x1E00, 0x1E01, 0x1EA0,
        0x1EA1, 0x1EA2, 0x1EA3, 0x1EA4, 0x1EA5, 0x1EA6, 0x1EA7, 0x1EA8, 0x1EA9, 0x1EAA, 0x1EAB, 0x1EAC, 0x1EAD, 0x1EAE, 0x1EAF, 0x1EB0, 0x1EB1, 0x1EB2, 0x1EB3, 0x1EB4, 0x1EB5, 0x1EB6, 0x1EB7,
        0x212B, 0x24B6, 0x24D0, 0xFF21, 0xFF41, 0x0226, 0x0227, 0x01D400, 0x01D41A, 0x01D434, 0x01D44E, 0x01D468, 0x01D482, 0x01D49C, 0x01D4B6, 0x01D4D0, 0x01D4EA, 0x01D504, 0x01D51E,
        0x01D538, 0x01D552, 0x01D56C, 0x01D586, 0x01D5A0, 0x01D5BA, 0x01D5D4, 0x01D5EE, 0x01D608, 0x01D622, 0x01D63C, 0x01D656, 0x01D670, 0x01D68A, 0x1D2C, 0x1D43, 0x2090, 0x01F130, 0x1E02,
        0x1E03, 0x1E04, 0x1E05, 0x1E06, 0x1E07, 0x212C, 0x24B7, 0x24D1, 0xFF22, 0xFF42, 0x01D401, 0x01D41B, 0x01D435, 0x01D44F, 0x01D469, 0x01D483, 0x01D4B7, 0x01D4D1, 0x01D4EB, 0x01D505,
        0x01D51F, 0x01D539, 0x01D553, 0x01D56D, 0x01D587, 0x01D5A1, 0x01D5BB, 0x01D5D5, 0x01D5EF, 0x01D609, 0x01D623, 0x01D63D, 0x01D657, 0x01D671, 0x01D68B, 0x1D2E, 0x1D47, 0x01F131,
        0x00C7, 0x00E7, 0x0106, 0x0107, 0x0108, 0x0109, 0x010A, 0x010B, 0x010C, 0x010D, 0x1E08, 0x1E09, 0x2102, 0x212D, 0x216D, 0x217D, 0x24B8, 0x24D2, 0xFF23, 0xFF43, 0x01D402, 0x01D41C,
        0x01D436, 0x01D450, 0x01D46A, 0x01D484, 0x01D49E, 0x01D4B8, 0x01D4D2, 0x01D4EC, 0x01D520, 0x01D554, 0x01D56E, 0x01D588, 0x01D5A2, 0x01D5BC, 0x01D5D6, 0x01D5F0, 0x01D60A, 0x01D624,
        0x01D63E, 0x01D658, 0x01D672, 0x01D68C, 0x1D9C, 0x01F12B, 0x01F132, 0x010E, 0x010F, 0x1E0A, 0x1E0B, 0x1E0C, 0x1E0D, 0x1E0E, 0x1E0F, 0x1E10, 0x1E11, 0x1E12, 0x1E13, 0x216E, 0x217E,
        0x24B9, 0x24D3, 0xFF24, 0xFF44, 0x2145, 0x2146, 0x01D403, 0x01D41D, 0x01D437, 0x01D451, 0x01D46B, 0x01D485, 0x01D49F, 0x01D4B9, 0x01D4D3, 0x01D4ED, 0x01D507, 0x01D521, 0x01D53B,
        0x01D555, 0x01D56F, 0x01D589, 0x01D5A3, 0x01D5BD, 0x01D5D7, 0x01D5F1, 0x01D60B, 0x01D625, 0x01D63F, 0x01D659, 0x01D673, 0x01D68D, 0x1D30, 0x1D48, 0x01F133, 0x00C8, 0x00C9, 0x00CA,
        0x00CB, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x0112, 0x0113, 0x0114, 0x0115, 0x0116, 0x0117, 0x0118, 0x0119, 0x011A, 0x011B, 0x0204, 0x0205, 0x0206, 0x0207, 0x1E14, 0x1E15, 0x1E16, 0x1E17,
        0x1E18, 0x1E19, 0x1E1A, 0x1E1B, 0x1E1C, 0x1E1D, 0x1EB8, 0x1EB9, 0x1EBA, 0x1EBB, 0x1EBC, 0x1EBD, 0x1EBE, 0x1EBF, 0x1EC0, 0x1EC1, 0x1EC2, 0x1EC3, 0x1EC4, 0x1EC5, 0x1EC6, 0x1EC7, 0x212F,
        0x2130, 0x24BA, 0x24D4, 0xFF25, 0xFF45, 0x0228, 0x0229, 0x2147, 0x01D404, 0x01D41E, 0x01D438, 0x01D452, 0x01D46C, 0x01D486, 0x01D4D4, 0x01D4EE, 0x01D508, 0x01D522, 0x01D53C, 0x01D556,
        0x01D570, 0x01D58A, 0x01D5A4, 0x01D5BE, 0x01D5D8, 0x01D5F2, 0x01D60C, 0x01D626, 0x01D640, 0x01D65A, 0x01D674, 0x01D68E, 0x1D31, 0x1D49, 0x2091, 0x01F134, 0x1E1E, 0x1E1F, 0x2131,
        0x24BB, 0x24D5, 0xFF26, 0xFF46, 0x01D405, 0x01D41F, 0x01D439, 0x01D453, 0x01D46D, 0x01D487, 0x01D4BB, 0x01D4D5, 0x01D4EF, 0x01D509, 0x01D523, 0x01D53D, 0x01D557, 0x01D571, 0x01D58B,
        0x01D5A5, 0x01D5BF, 0x01D5D9, 0x01D5F3, 0x01D60D, 0x01D627, 0x01D641, 0x01D65B, 0x01D675, 0x01D68F, 0x1DA0, 0x01F135, 0x011C, 0x011D, 0x011E, 0x011F, 0x0120, 0x0121, 0x0122, 0x0123,
        0x01E6, 0x01E7, 0x01F4, 0x01F5, 0x1E20, 0x1E21, 0x210A, 0x24BC, 0x24D6, 0xFF27, 0xFF47, 0x01D406, 0x01D420, 0x01D43A, 0x01D454, 0x01D46E, 0x01D488, 0x01D4A2, 0x01D4D6, 0x01D4F0,
        0x01D50A, 0x01D524, 0x01D53E, 0x01D558, 0x01D572, 0x01D58C, 0x01D5A6, 0x01D5C0, 0x01D5DA, 0x01D5F4, 0x01D60E, 0x01D628, 0x01D642, 0x01D65C, 0x01D676, 0x01D690, 0x1D33, 0x1D4D,
        0x01F136, 0x0124, 0x0125, 0x02B0, 0x1E22, 0x1E23, 0x1E24, 0x1E25, 0x1E26, 0x1E27, 0x1E28, 0x1E29, 0x1E2A, 0x1E2B, 0x1E96, 0x210B, 0x210C, 0x210D, 0x210E, 0x24BD, 0x24D7, 0xFF28, 0xFF48,
        0x021E, 0x021F, 0x01D407, 0x01D421, 0x01D43B, 0x01D46F, 0x01D489, 0x01D4BD, 0x01D4D7, 0x01D4F1, 0x01D525, 0x01D559, 0x01D573, 0x01D58D, 0x01D5A7, 0x01D5C1, 0x01D5DB, 0x01D5F5,
        0x01D60F, 0x01D629, 0x01D643, 0x01D65D, 0x01D677, 0x01D691, 0x1D34, 0x2095, 0x01F137, 0x00CC, 0x00CD, 0x00CE, 0x00CF, 0x00EC, 0x00ED, 0x00EE, 0x00EF, 0x0128, 0x0129, 0x012A, 0x012B,
        0x012C, 0x012D, 0x012E, 0x012F, 0x0130, 0x01CF, 0x01D0, 0x0208, 0x0209, 0x020A, 0x020B, 0x1E2C, 0x1E2D, 0x1E2E, 0x1E2F, 0x1EC8, 0x1EC9, 0x1ECA, 0x1ECB, 0x2110, 0x2111, 0x2160, 0x2170,
        0x24BE, 0x24D8, 0xFF29, 0xFF49, 0x2071, 0x2139, 0x2148, 0x01D408, 0x01D422, 0x01D43C, 0x01D456, 0x01D470, 0x01D48A, 0x01D4BE, 0x01D4D8, 0x01D4F2, 0x01D526, 0x01D540, 0x01D55A,
        0x01D574, 0x01D58E, 0x01D5A8, 0x01D5C2, 0x01D5DC, 0x01D5F6, 0x01D610, 0x01D62A, 0x01D644, 0x01D65E, 0x01D678, 0x01D692, 0x1D35, 0x1D62, 0x01F138, 0x0134, 0x0135, 0x01F0, 0x02B2,
        0x24BF, 0x24D9, 0xFF2A, 0xFF4A, 0x2149, 0x01D409, 0x01D423, 0x01D43D, 0x01D457, 0x01D471, 0x01D48B, 0x01D4A5, 0x01D4BF, 0x01D4D9, 0x01D4F3, 0x01D50D, 0x01D527, 0x01D541, 0x01D55B,
        0x01D575, 0x01D58F, 0x01D5A9, 0x01D5C3, 0x01D5DD, 0x01D5F7, 0x01D611, 0x01D62B, 0x01D645, 0x01D65F, 0x01D679, 0x01D693, 0x1D36, 0x2C7C, 0x01F139, 0x0136, 0x0137, 0x01E8, 0x01E9,
        0x1E30, 0x1E31, 0x1E32, 0x1E33, 0x1E34, 0x1E35, 0x212A, 0x24C0, 0x24DA, 0xFF2B, 0xFF4B, 0x01D40A, 0x01D424, 0x01D43E, 0x01D458, 0x01D472, 0x01D48C, 0x01D4A6, 0x01D4C0, 0x01D4DA,
        0x01D4F4, 0x01D50E, 0x01D528, 0x01D542, 0x01D55C, 0x01D576, 0x01D590, 0x01D5AA, 0x01D5C4, 0x01D5DE, 0x01D5F8, 0x01D612, 0x01D62C, 0x01D646, 0x01D660, 0x01D67A, 0x01D694, 0x1D37,
        0x1D4F, 0x2096, 0x01F13A, 0x0139, 0x013A, 0x013B, 0x013C, 0x013D, 0x013E, 0x02E1, 0x1E36, 0x1E37, 0x1E38, 0x1E39, 0x1E3A, 0x1E3B, 0x1E3C, 0x1E3D, 0x2112, 0x2113, 0x216C, 0x217C, 0x24C1,
        0x24DB, 0xFF2C, 0xFF4C, 0x01D40B, 0x01D425, 0x01D43F, 0x01D459, 0x01D473, 0x01D48D, 0x01D4DB, 0x01D4F5, 0x01D50F, 0x01D529, 0x01D543, 0x01D55D, 0x01D577, 0x01D591, 0x01D5AB, 0x01D5C5,
        0x01D5DF, 0x01D5F9, 0x01D613, 0x01D62D, 0x01D647, 0x01D661, 0x01D67B, 0x01D695, 0x1D38, 0x01D4C1, 0x2097, 0x01F13B, 0x1E3E, 0x1E3F, 0x1E40, 0x1E41, 0x1E42, 0x1E43, 0x2133, 0x216F,
        0x217F, 0x24C2, 0x24DC, 0xFF2D, 0xFF4D, 0x01D40C, 0x01D426, 0x01D440, 0x01D45A, 0x01D474, 0x01D48E, 0x01D4C2, 0x01D4DC, 0x01D4F6, 0x01D510, 0x01D52A, 0x01D544, 0x01D55E, 0x01D578,
        0x01D592, 0x01D5AC, 0x01D5C6, 0x01D5E0, 0x01D5FA, 0x01D614, 0x01D62E, 0x01D648, 0x01D662, 0x01D67C, 0x01D696, 0x1D39, 0x1D50, 0x2098, 0x01F13C, 0x00D1, 0x00F1, 0x0143, 0x0144,
        0x0145, 0x0146, 0x0147, 0x0148, 0x1E44, 0x1E45, 0x1E46, 0x1E47, 0x1E48, 0x1E49, 0x1E4A, 0x1E4B, 0x207F, 0x2115, 0x24C3, 0x24DD, 0xFF2E, 0xFF4E, 0x01F8, 0x01F9, 0x01D40D, 0x01D427,
        0x01D441, 0x01D45B, 0x01D475, 0x01D48F, 0x01D4A9, 0x01D4C3, 0x01D4DD, 0x01D4F7, 0x01D511, 0x01D52B, 0x01D55F, 0x01D579, 0x01D593, 0x01D5AD, 0x01D5C7, 0x01D5E1, 0x01D5FB, 0x01D615,
        0x01D62F, 0x01D649, 0x01D663, 0x01D67D, 0x01D697, 0x1D3A, 0x01F13D, 0x2099, 0x00BA, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x014C, 0x014D,
        0x014E, 0x014F, 0x0150, 0x0151, 0x01A0, 0x01A1, 0x01D1, 0x01D2, 0x01EA, 0x01EB, 0x01EC, 0x01ED, 0x020C, 0x020D, 0x020E, 0x020F, 0x1E4C, 0x1E4D, 0x1E4E, 0x1E4F, 0x1E50, 0x1E51, 0x1E52,
        0x1E53, 0x1ECC, 0x1ECD, 0x1ECE, 0x1ECF, 0x1ED0, 0x1ED1, 0x1ED2, 0x1ED3, 0x1ED4, 0x1ED5, 0x1ED6, 0x1ED7, 0x1ED8, 0x1ED9, 0x1EDA, 0x1EDB, 0x1EDC, 0x1EDD, 0x1EDE, 0x1EDF, 0x1EE0, 0x1EE1,
        0x1EE2, 0x1EE3, 0x2134, 0x24C4, 0x24DE, 0xFF2F, 0xFF4F, 0x022A, 0x022B, 0x022C, 0x022D, 0x022E, 0x022F, 0x0230, 0x0231, 0x01D40E, 0x01D428, 0x01D442, 0x01D45C, 0x01D476, 0x01D490,
        0x01D4AA, 0x01D4DE, 0x01D4F8, 0x01D512, 0x01D52C, 0x01D546, 0x01D560, 0x01D57A, 0x01D594, 0x01D5AE, 0x01D5C8, 0x01D5E2, 0x01D5FC, 0x01D616, 0x01D630, 0x01D64A, 0x01D664, 0x01D67E,
        0x01D698, 0x1D3C, 0x1D52, 0x2092, 0x01F13E, 0x1E54, 0x1E55, 0x1E56, 0x1E57, 0x2119, 0x24C5, 0x24DF, 0xFF30, 0xFF50, 0x01D40F, 0x01D429, 0x01D443, 0x01D45D, 0x01D477, 0x01D491, 0x01D4AB,
        0x01D4C5, 0x01D4DF, 0x01D4F9, 0x01D513, 0x01D52D, 0x01D561, 0x01D57B, 0x01D595, 0x01D5AF, 0x01D5C9, 0x01D5E3, 0x01D5FD, 0x01D617, 0x01D631, 0x01D64B, 0x01D665, 0x01D67F, 0x01D699,
        0x1D3E, 0x1D56, 0x01F13F, 0x209A, 0x211A, 0x24C6, 0x24E0, 0xFF31, 0xFF51, 0x01D410, 0x01D42A, 0x01D444, 0x01D45E, 0x01D478, 0x01D492, 0x01D4AC, 0x01D4C6, 0x01D4E0, 0x01D4FA, 0x01D514,
        0x01D52E, 0x01D562, 0x01D57C, 0x01D596, 0x01D5B0, 0x01D5CA, 0x01D5E4, 0x01D5FE, 0x01D618, 0x01D632, 0x01D64C, 0x01D666, 0x01D680, 0x01D69A, 0x01F140, 0x0154, 0x0155, 0x0156, 0x0157,
        0x0158, 0x0159, 0x0210, 0x0211, 0x0212, 0x0213, 0x02B3, 0x1E58, 0x1E59, 0x1E5A, 0x1E5B, 0x1E5C, 0x1E5D, 0x1E5E, 0x1E5F, 0x211B, 0x211C, 0x211D, 0x24C7, 0x24E1, 0xFF32, 0xFF52, 0x01D411,
        0x01D42B, 0x01D445, 0x01D45F, 0x01D479, 0x01D493, 0x01D4C7, 0x01D4E1, 0x01D4FB, 0x01D52F, 0x01D563, 0x01D57D, 0x01D597, 0x01D5B1, 0x01D5CB, 0x01D5E5, 0x01D5FF, 0x01D619, 0x01D633,
        0x01D64D, 0x01D667, 0x01D681, 0x01D69B, 0x1D3F, 0x1D63, 0x01F12C, 0x01F141, 0x015A, 0x015B, 0x015C, 0x015D, 0x015E, 0x015F, 0x0160, 0x0161, 0x017F, 0x02E2, 0x1E60, 0x1E61, 0x1E62,
        0x1E63, 0x1E64, 0x1E65, 0x1E66, 0x1E67, 0x1E68, 0x1E69, 0x24C8, 0x24E2, 0xFF33, 0xFF53, 0x1E9B, 0x0218, 0x0219, 0x01D412, 0x01D42C, 0x01D446, 0x01D460, 0x01D47A, 0x01D494, 0x01D4AE,
        0x01D4C8, 0x01D4E2, 0x01D4FC, 0x01D516, 0x01D530, 0x01D54A, 0x01D564, 0x01D57E, 0x01D598, 0x01D5B2, 0x01D5CC, 0x01D5E6, 0x01D600, 0x01D61A, 0x01D634, 0x01D64E, 0x01D668, 0x01D682,
        0x01D69C, 0x01F142, 0x209B, 0x0162, 0x0163, 0x0164, 0x0165, 0x1E6A, 0x1E6B, 0x1E6C, 0x1E6D, 0x1E6E, 0x1E6F, 0x1E70, 0x1E71, 0x1E97, 0x24C9, 0x24E3, 0xFF34, 0xFF54, 0x021A, 0x021B,
        0x01D413, 0x01D42D, 0x01D447, 0x01D461, 0x01D47B, 0x01D495, 0x01D4AF, 0x01D4C9, 0x01D4E3, 0x01D4FD, 0x01D517, 0x01D531, 0x01D54B, 0x01D565, 0x01D57F, 0x01D599, 0x01D5B3, 0x01D5CD,
        0x01D5E7, 0x01D601, 0x01D61B, 0x01D635, 0x01D64F, 0x01D669, 0x01D683, 0x01D69D, 0x1D40, 0x1D57, 0x209C, 0x01F143, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00F9, 0x00FA, 0x00FB, 0x00FC,
        0x0168, 0x0169, 0x016A, 0x016B, 0x016C, 0x016D, 0x016E, 0x016F, 0x0170, 0x0171, 0x0172, 0x0173, 0x01AF, 0x01B0, 0x01D3, 0x01D4, 0x01D5, 0x01D6, 0x01D7, 0x01D8, 0x01D9, 0x01DA, 0x01DB,
        0x01DC, 0x0214, 0x0215, 0x0216, 0x0217, 0x1E72, 0x1E73, 0x1E74, 0x1E75, 0x1E76, 0x1E77, 0x1E78, 0x1E79, 0x1E7A, 0x1E7B, 0x1EE4, 0x1EE5, 0x1EE6, 0x1EE7, 0x1EE8, 0x1EE9, 0x1EEA, 0x1EEB,
        0x1EEC, 0x1EED, 0x1EEE, 0x1EEF, 0x1EF0, 0x1EF1, 0x24CA, 0x24E4, 0xFF35, 0xFF55, 0x01D414, 0x01D42E, 0x01D448, 0x01D462, 0x01D47C, 0x01D496, 0x01D4B0, 0x01D4CA, 0x01D4E4, 0x01D4FE,
        0x01D518, 0x01D532, 0x01D54C, 0x01D566, 0x01D580, 0x01D59A, 0x01D5B4, 0x01D5CE, 0x01D5E8, 0x01D602, 0x01D61C, 0x01D636, 0x01D650, 0x01D66A, 0x01D684, 0x01D69E, 0x1D41, 0x1D58,
        0x1D64, 0x01F144, 0x1E7C, 0x1E7D, 0x1E7E, 0x1E7F, 0x2164, 0x2174, 0x24CB, 0x24E5, 0xFF36, 0xFF56, 0x01D415, 0x01D42F, 0x01D449, 0x01D463, 0x01D47D, 0x01D497, 0x01D4B1, 0x01D4CB,
        0x01D4E5, 0x01D4FF, 0x01D519, 0x01D533, 0x01D54D, 0x01D567, 0x01D581, 0x01D59B, 0x01D5B5, 0x01D5CF, 0x01D5E9, 0x01D603, 0x01D61D, 0x01D637, 0x01D651, 0x01D66B, 0x01D685, 0x01D69F,
        0x1D5B, 0x1D65, 0x2C7D, 0x01F145, 0x0174, 0x0175, 0x02B7, 0x1E80, 0x1E81, 0x1E82, 0x1E83, 0x1E84, 0x1E85, 0x1E86, 0x1E87, 0x1E88, 0x1E89, 0x1E98, 0x24CC, 0x24E6, 0xFF37, 0xFF57, 0x01D416,
        0x01D430, 0x01D44A, 0x01D464, 0x01D47E, 0x01D498, 0x01D4B2, 0x01D4CC, 0x01D4E6, 0x01D500, 0x01D51A, 0x01D534, 0x01D54E, 0x01D568, 0x01D582, 0x01D59C, 0x01D5B6, 0x01D5D0, 0x01D5EA,
        0x01D604, 0x01D61E, 0x01D638, 0x01D652, 0x01D66C, 0x01D686, 0x01D6A0, 0x1D42, 0x01F146, 0x02E3, 0x1E8A, 0x1E8B, 0x1E8C, 0x1E8D, 0x2169, 0x2179, 0x24CD, 0x24E7, 0xFF38, 0xFF58, 0x01D417,
        0x01D431, 0x01D44B, 0x01D465, 0x01D47F, 0x01D499, 0x01D4B3, 0x01D4CD, 0x01D4E7, 0x01D501, 0x01D51B, 0x01D535, 0x01D54F, 0x01D569, 0x01D583, 0x01D59D, 0x01D5B7, 0x01D5D1, 0x01D5EB,
        0x01D605, 0x01D61F, 0x01D639, 0x01D653, 0x01D66D, 0x01D687, 0x01D6A1, 0x2093, 0x01F147, 0x00DD, 0x00FD, 0x00FF, 0x0176, 0x0177, 0x0178, 0x02B8, 0x1E8E, 0x1E8F, 0x1E99, 0x1EF2, 0x1EF3,
        0x1EF4, 0x1EF5, 0x1EF6, 0x1EF7, 0x1EF8, 0x1EF9, 0x24CE, 0x24E8, 0xFF39, 0xFF59, 0x0232, 0x0233, 0x01D418, 0x01D432, 0x01D44C, 0x01D466, 0x01D480, 0x01D49A, 0x01D4B4, 0x01D4CE, 0x01D4E8,
        0x01D502, 0x01D51C, 0x01D536, 0x01D550, 0x01D56A, 0x01D584, 0x01D59E, 0x01D5B8, 0x01D5D2, 0x01D5EC, 0x01D606, 0x01D620, 0x01D63A, 0x01D654, 0x01D66E, 0x01D688, 0x01D6A2, 0x01F148,
        0x0179, 0x017A, 0x017B, 0x017C, 0x017D, 0x017E, 0x1E90, 0x1E91, 0x1E92, 0x1E93, 0x1E94, 0x1E95, 0x2124, 0x2128, 0x24CF, 0x24E9, 0xFF3A, 0xFF5A, 0x01D419, 0x01D433, 0x01D44D, 0x01D467,
        0x01D481, 0x01D49B, 0x01D4B5, 0x01D4CF, 0x01D4E9, 0x01D503, 0x01D537, 0x01D56B, 0x01D585, 0x01D59F, 0x01D5B9, 0x01D5D3, 0x01D5ED, 0x01D607, 0x01D621, 0x01D63B, 0x01D655, 0x01D66F,
        0x01D689, 0x01D6A3, 0x1DBB, 0x01F149, 0x0020, 0x00A0, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008, 0x2009, 0x200A, 0x2028, 0x2029, 0x3000, 0x1680, 0x180E,
        0x202F, 0x205F, 
    );
    protected static $_offsets = array(
        /*      START | 1.1.0 | 1.1.5 | 2.0.0 | 2.1.0 | 2.1.2 | 2.1.5 | 2.1.8 | 2.1.9 | 3.0.0 | 3.0.1 | 3.1.0 | 3.1.1 | 3.2.0 | 4.0.0 | 4.0.1 | 4.1.0 | 5.0.0 | 5.1.0 | 5.2.0 | 6.0.0 */
        /* 0 */
        array(      0,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      9,      9,      9,      9,      9,      9,      9,      9,      9,      9 ),
        /* 1 */
        array(      9,     13,     13,     13,     13,     13,     13,     13,     13,     13,     13,     18,     18,     18,     18,     18,     18,     18,     18,     18,     18 ),
        /* 2 */
        array(     18,     22,     22,     22,     22,     22,     22,     22,     22,     22,     22,     27,     27,     27,     27,     27,     27,     27,     27,     27,     27 ),
        /* 3 */
        array(     27,     31,     31,     31,     31,     31,     31,     31,     31,     31,     31,     36,     36,     36,     36,     36,     36,     36,     36,     36,     36 ),
        /* 4 */
        array(     36,     40,     40,     40,     40,     40,     40,     40,     40,     40,     40,     45,     45,     45,     45,     45,     45,     45,     45,     45,     45 ),
        /* 5 */
        array(     45,     49,     49,     49,     49,     49,     49,     49,     49,     49,     49,     54,     54,     54,     54,     54,     54,     54,     54,     54,     54 ),
        /* 6 */
        array(     54,     58,     58,     58,     58,     58,     58,     58,     58,     58,     58,     63,     63,     63,     63,     63,     63,     63,     63,     63,     63 ),
        /* 7 */
        array(     63,     67,     67,     67,     67,     67,     67,     67,     67,     67,     67,     72,     72,     72,     72,     72,     72,     72,     72,     72,     72 ),
        /* 8 */
        array(     72,     76,     76,     76,     76,     76,     76,     76,     76,     76,     76,     81,     81,     81,     81,     81,     81,     81,     81,     81,     81 ),
        /* 9 */
        array(     81,     85,     85,     85,     85,     85,     85,     85,     85,     85,     85,     90,     90,     90,     90,     90,     90,     90,     90,     90,     90 ),
        /* a */
        array(     90,    152,    152,    152,    152,    152,    152,    152,    152,    154,    154,    180,    180,    180,    182,    182,    183,    183,    183,    183,    184 ),
        /* b */
        array(    184,    195,    195,    195,    195,    195,    195,    195,    195,    195,    195,    220,    220,    220,    222,    222,    222,    222,    222,    223,    223 ),
        /* c */
        array(    223,    243,    243,    243,    243,    243,    243,    243,    243,    243,    243,    267,    267,    267,    267,    267,    268,    268,    268,    269,    270 ),
        /* d */
        array(    270,    288,    288,    288,    288,    288,    288,    288,    288,    288,    288,    314,    314,    316,    318,    318,    318,    318,    318,    318,    319 ),
        /* e */
        array(    319,    373,    373,    373,    373,    373,    373,    373,    373,    375,    375,    399,    399,    400,    402,    402,    403,    403,    403,    403,    404 ),
        /* f */
        array(    404,    411,    411,    411,    411,    411,    411,    411,    411,    411,    411,    436,    436,    436,    436,    436,    437,    437,    437,    437,    438 ),
        /* g */
        array(    438,    457,    457,    457,    457,    457,    457,    457,    457,    457,    457,    482,    482,    482,    484,    484,    484,    484,    484,    484,    485 ),
        /* h */
        array(    485,    507,    507,    507,    507,    507,    507,    507,    507,    509,    509,    531,    531,    531,    532,    532,    532,    532,    532,    532,    534 ),
        /* i */
        array(    534,    573,    573,    573,    573,    573,    573,    573,    573,    574,    574,    598,    598,    600,    602,    602,    602,    602,    602,    602,    603 ),
        /* j */
        array(    603,    611,    611,    611,    611,    611,    611,    611,    611,    611,    611,    637,    637,    638,    639,    639,    639,    639,    640,    640,    641 ),
        /* k */
        array(    641,    656,    656,    656,    656,    656,    656,    656,    656,    656,    656,    682,    682,    682,    684,    684,    684,    684,    684,    684,    686 ),
        /* l */
        array(    686,    709,    709,    709,    709,    709,    709,    709,    709,    709,    709,    733,    733,    733,    735,    735,    735,    735,    735,    735,    737 ),
        /* m */
        array(    737,    750,    750,    750,    750,    750,    750,    750,    750,    750,    750,    775,    775,    775,    777,    777,    777,    777,    777,    777,    779 ),
        /* n */
        array(    779,    801,    801,    801,    801,    801,    801,    801,    801,    803,    803,    828,    828,    828,    829,    829,    829,    829,    829,    830,    831 ),
        /* o */
        array(    831,    897,    897,    897,    897,    897,    897,    897,    897,    905,    905,    930,    930,    930,    932,    932,    933,    933,    933,    933,    934 ),
        /* p */
        array(    934,    943,    943,    943,    943,    943,    943,    943,    943,    943,    943,    968,    968,    968,    970,    970,    970,    970,    970,    971,    972 ),
        /* q */
        array(    972,    977,    977,    977,    977,    977,    977,    977,    977,    977,    977,   1002,   1002,   1002,   1002,   1002,   1002,   1002,   1002,   1002,   1003 ),
        /* r */
        array(   1003,   1029,   1029,   1029,   1029,   1029,   1029,   1029,   1029,   1029,   1029,   1052,   1052,   1052,   1054,   1054,   1054,   1054,   1054,   1055,   1056 ),
        /* s */
        array(   1056,   1080,   1080,   1081,   1081,   1081,   1081,   1081,   1081,   1083,   1083,   1109,   1109,   1109,   1109,   1109,   1109,   1109,   1109,   1110,   1111 ),
        /* t */
        array(   1111,   1128,   1128,   1128,   1128,   1128,   1128,   1128,   1128,   1130,   1130,   1156,   1156,   1156,   1158,   1158,   1158,   1158,   1158,   1158,   1160 ),
        /* u */
        array(   1160,   1224,   1224,   1224,   1224,   1224,   1224,   1224,   1224,   1224,   1224,   1250,   1250,   1250,   1253,   1253,   1253,   1253,   1253,   1253,   1254 ),
        /* v */
        array(   1254,   1264,   1264,   1264,   1264,   1264,   1264,   1264,   1264,   1264,   1264,   1290,   1290,   1290,   1292,   1292,   1292,   1292,   1293,   1293,   1294 ),
        /* w */
        array(   1294,   1312,   1312,   1312,   1312,   1312,   1312,   1312,   1312,   1312,   1312,   1338,   1338,   1338,   1339,   1339,   1339,   1339,   1339,   1340,   1340 ),
        /* x */
        array(   1340,   1351,   1351,   1351,   1351,   1351,   1351,   1351,   1351,   1351,   1351,   1377,   1377,   1377,   1377,   1377,   1378,   1378,   1378,   1378,   1379 ),
        /* y */
        array(   1379,   1401,   1401,   1401,   1401,   1401,   1401,   1401,   1401,   1403,   1403,   1429,   1429,   1429,   1429,   1429,   1429,   1429,   1429,   1429,   1430 ),
        /* z */
        array(   1430,   1448,   1448,   1448,   1448,   1448,   1448,   1448,   1448,   1448,   1448,   1472,   1472,   1472,   1472,   1472,   1473,   1473,   1473,   1473,   1474 ),
        /* ignorables */
        array(   1474,   1490,   1490,   1490,   1490,   1490,   1490,   1490,   1490,   1493,   1493,   1493,   1493,   1494,   1494,   1494,   1494,   1494,   1494,   1494,   1494 )
    );

    protected $_key;
    protected $_fakes;
    protected $_challenge;
    protected $_serializer;

    private $_attributes = array(
        self::ATTR_ALPHABET => '23456789abcdefghjkmnpqrstuvwxyz',
        self::ATTR_REVERSED => self::RANDOM,
        self::ATTR_NOISE_LENGTH => 2,
        self::ATTR_HTML_LETTER_TAG => 'span',
        self::ATTR_HTML_WRAPPER_TAG => 'div',
        self::ATTR_UNICODE_VERSION => self::UNICODE_6_0_0,
        self::ATTR_HTML_WRAPPER_ID => 'captcha',
        self::ATTR_CHALLENGE_LENGTH => 8,
        self::ATTR_SESSION_PREFIX => 'captcha_',
        self::ATTR_FAKE_CHARACTERS_LENGTH => 2,
        self::ATTR_FAKE_CHARACTERS_STYLE => 'display: none;',
        self::ATTR_FAKE_CHARACTERS_COLOR => self::COLOR_NONE,
        self::ATTR_SIGNIFICANT_CHARACTERS_STYLE => '',
        self::ATTR_SKIP_UNICODE_FOR_CHALLENGE => FALSE,
        self::ATTR_SIGNIFICANT_CHARACTERS_COLOR => self::COLOR_NONE,
    );

    private $_alphabet_len; // to avoid many strlen($this->_attributes[self::ATTR_ALPHABET])

    const UNINITIALIZED_CHAR = 0x81;
    const UNSIGNIFICANT_CHAR = 0x82;

    private static $colors = array(
        self::COLOR_RED => array(0, 30, 75, 100, 40, 60),
        self::COLOR_GREEN => array(90, 120, 75, 100, 40, 60),
        self::COLOR_BLUE => array(210, 240, 75, 100, 40, 60),
        self::COLOR_LIGHT => array(0, 359, 0, 50, 92, 100),
        self::COLOR_DARK => array(0, 359, 0, 100, 0, 6),
    );

    protected function generateChallenge()
    {
        $this->_fakes = array();
        if ($this->_attributes[self::ATTR_FAKE_CHARACTERS_LENGTH]) {
            $index = range(0, $this->_attributes[self::ATTR_CHALLENGE_LENGTH]);
            shuffle($index);
            for ($i = 0; $i < $this->_attributes[self::ATTR_FAKE_CHARACTERS_LENGTH]; $i++) {
                $this->_fakes[$index[$i]] = substr($this->_attributes[self::ATTR_ALPHABET], rand(0, $this->_alphabet_len - 1), 1);
            }
        }
        $this->_challenge = '';
        for ($i = 0; $i < $this->_attributes[self::ATTR_CHALLENGE_LENGTH]; $i++) {
            // can't use: $token .= $this->_attributes[self::ATTR_ALPHABET][rand(0, $this->_alphabet_len - 1)]; ?!?
            $this->_challenge .= substr($this->_attributes[self::ATTR_ALPHABET], rand(0, $this->_alphabet_len - 1), 1);
        }

        return $this->_challenge;
    }

    protected static function formatCP($cp)
    {
        return sprintf($cp > 0xFFFF ? '%06X' : '%04X', $cp);
    }

    protected function appendIgnorables(&$ret)
    {
        if ($this->_attributes[self::ATTR_NOISE_LENGTH]) {
            for ($i = rand(0, $this->_attributes[self::ATTR_NOISE_LENGTH]); $i > 0; $i--) {
                $ret .= '\\' . self::formatCP(self::$_table[rand(self::$_offsets[count(self::$_offsets)-1][0], self::$_offsets[count(self::$_offsets)-1][$this->_attributes[self::ATTR_UNICODE_VERSION]] - 1)]);
            }
        }
    }

    protected static function hue_to_rgb($m1, $m2, $h)
    {
        if ($h < 0) {
            $h += 360;
        }
        if ($h > 360) {
            $h -= 360;
        }
        if ($h < 60) {
            return ($m1 + ($m2 - $m1) * ($h / 60.0)) * 255.5;
        }
        if ($h < 180) {
            return ($m2 * 255.5);
        }
        if ($h < 240) {
            return ($m1 + ($m2 - $m1) * ((240 - $h) / 60.0)) * 255.5;
        }

        return $m1 * 255.5;
    }

    protected static function hsl_to_rgb($h, $s, $l)
    {
        if ($l == 0) {
            $r = $g = $b = 0;
        } else {
            $s /= 100;
            $l /= 100;
            if ($l <= 0.5) {
                $m2 = $l * ($s + 1);
            } else {
                $m2 = $l + $s - $l * $s;
            }
            $m1 = $l * 2 - $m2;
            $r = self::hue_to_rgb($m1, $m2, $h + 120);
            $g = self::hue_to_rgb($m1, $m2, $h);
            $b = self::hue_to_rgb($m1, $m2, $h - 120);
        }

        return array($r, $g, $b);
    }

    protected function setColor($is_significant)
    {
        if ($color_key = $this->_attributes[$is_significant ? CSSCaptcha::ATTR_SIGNIFICANT_CHARACTERS_COLOR : CSSCaptcha::ATTR_FAKE_CHARACTERS_COLOR]) {
            $color = self::$colors[$color_key];
            return vsprintf('color: #%02X%02X%02X; ', self::hsl_to_rgb(rand($color[0], $color[1]), rand($color[2], $color[3]), rand($color[4], $color[5])));
        } else {
            return '';
        }
    }

    protected function renew()
    {
        $this->_serializer->set(
            $this->_attributes[self::ATTR_SESSION_PREFIX] . $this->_key,
            array(
                'challenge' => $this->_challenge = $this->generateChallenge(),
                'fakes' => $this->_fakes
            )
        );
    }

    public function __construct($key, CSSCaptchaStoreInterface $serializer, $attributes = array())
    {
        $this->_key = $key;
        $this->_serializer = $serializer;
        foreach ($attributes as $k => $v) {
            $this->setAttribute($k, $v);
        }
        $this->_alphabet_len = strlen($this->_attributes[self::ATTR_ALPHABET]);
        $data = $this->_serializer->get($this->_attributes[self::ATTR_SESSION_PREFIX] . $this->_key);
        $this->_challenge = isset($data['challenge']) ? $data['challenge'] : NULL;
        $this->_fakes = isset($data['fakes']) ? $data['fakes'] : NULL;
        if (
               !$this->_challenge
            || !preg_match('.^[' . preg_quote($this->_attributes[self::ATTR_ALPHABET]) . ']+$.D', $this->_challenge)
        ) {
            $this->renew();
        }
    }

    protected function generateChar(/*&$ret,*/ $index, $character, $significant, $reversed)
    {
        $ret = '';
        $p = intval($character, 36);
        if ($reversed) {
            $ret .= sprintf(
                "#%s %s:nth-child(%s%d) { order: %d; }\n",
                $this->_attributes[self::ATTR_HTML_WRAPPER_ID],
                $this->_attributes[self::ATTR_HTML_LETTER_TAG],
                rand(0, 1) ? '0n+' : '',
                $index + 1,
                strlen($this->_challenge) - $index
            );
        }
        $ret .= '#' . $this->_attributes[self::ATTR_HTML_WRAPPER_ID] . ' ' . $this->_attributes[self::ATTR_HTML_LETTER_TAG] . ':nth-child(';
        if (rand(0, 1)) {
            $ret .= '0n+';
        }
        $ret .= $index + 1;
        $ret .= '):after { content: "';
        $this->appendIgnorables($ret);
        $ret .= '\\';
        if ($this->_attributes[self::ATTR_SKIP_UNICODE_FOR_CHALLENGE]) {
            $ret .= sprintf('%04X', ord($character));
        } else {
            $ret .= self::formatCP(self::$_table[rand(self::$_offsets[$p][0], self::$_offsets[$p][$this->_attributes[self::ATTR_UNICODE_VERSION]] - 1)]);
        }
        $this->appendIgnorables($ret);
        $ret .= '"; ';
        $ret .= $this->setColor($significant);
        if ($significant) {
            $ret .= $this->_attributes[self::ATTR_SIGNIFICANT_CHARACTERS_STYLE];
        } else {
            $ret .= $this->_attributes[self::ATTR_FAKE_CHARACTERS_STYLE];
        }
        $ret .= " }\n";

        return $ret;
    }

    public function render($what = 0x11/*self::RENDER_CSS | self::RENDER_HTML*/)
    {
        $ret = '';
        $total_len = strlen($this->_challenge) + count($this->_fakes);

//         $rtl = ($what == self::RENDER_CSS | self::RENDER_HTML) && !$this->_attributes[self::ATTR_ONLY_LTR] && rand(0, 1); # TODO: remove $what == self::RENDER_CSS | self::RENDER_HTML test, implies to move "$rtl" to a higher "scope" (session and/or attribute)

        if ($what & self::RENDER_CSS) {
            $reversed = self::ALWAYS == $this->_attributes[self::ATTR_REVERSED] || (self::RANDOM == $this->_attributes[self::ATTR_REVERSED] && rand(0, 1));
            if ($what & self::RENDER_HTML) {
                $ret .= '<style type="text/css">' . "\n";
            }
            if ($reversed) {
                $ret .= sprintf("#%s { display: flex; flex-direction: row-reverse; }\n", $this->_attributes[self::ATTR_HTML_WRAPPER_ID]);
            }
            $index = range(0, $total_len - 1);
            shuffle($index);
            if ($this->_fakes) {
                $map = array_fill(0, $total_len, self::UNINITIALIZED_CHAR);
                foreach ($this->_fakes as $k => $v) {
                    $map[$k] = self::UNSIGNIFICANT_CHAR;
                }
                for ($i = $j = 0; $i < strlen($this->_challenge); $j++) {
                    if (self::UNINITIALIZED_CHAR == $map[$j]) {
                        $map[$j] = $i++;
                    }
                }
            } else {
                $map = range(0, $total_len - 1);
            }
//             if ($rtl) {
//                 $ret .= '#' . $this->_attributes[self::ATTR_HTML_WRAPPER_ID] . ' { float: left; /*position: absolute; left: 0;*/ height: auto; overflow: hidden; zoom: 1; }' . "\n";
//                 $ret .= '#' . $this->_attributes[self::ATTR_HTML_WRAPPER_ID] . ' span { float: right; }' . "\n";
//                 $ret .= '#' . $this->_attributes[self::ATTR_HTML_WRAPPER_ID] . ':after { content: "."; visibility: hidden; display: block; height: 0; clear: both; }' . "\n";
//                 $map = array_reverse($map);
//             }
            for ($i = 0; $i < $total_len; $i++) {
                if (self::UNSIGNIFICANT_CHAR == $map[$index[$i]]) {
                    $ret .= $this->generateChar($index[$i], $this->_fakes[/*$rtl ? $total_len - 1 - $index[$i] : */$index[$i]], FALSE, $reversed);
                } else {
                    $ret .= $this->generateChar($index[$i], $this->_challenge[$map[$index[$i]]], TRUE, $reversed);
                }
            }
            if ($what & self::RENDER_HTML) {
                $ret .= '</style>';
            }
        }

        if ($what & self::RENDER_HTML) {
            $ret .= '<' . $this->_attributes[self::ATTR_HTML_WRAPPER_TAG] . ' id="' . $this->_attributes[self::ATTR_HTML_WRAPPER_ID] . '">'
                 . str_repeat('<' . $this->_attributes[self::ATTR_HTML_LETTER_TAG] . '></' . $this->_attributes[self::ATTR_HTML_LETTER_TAG] . '>', $total_len)
                 . '</' . $this->_attributes[self::ATTR_HTML_WRAPPER_TAG] . '>';
        }

        return $ret;
    }

    public function validate($user_input)
    {
        $match = $this->_challenge === $user_input;
        $this->renew();

        return $match;
    }

    public function cleanup()
    {
        $this->_serializer->remove($this->_attributes[self::ATTR_SESSION_PREFIX] . $this->_key);
    }

    public function getKey()
    {
        return $this->_key;
    }

    public function getChallenge()
    {
        return $this->_challenge;
    }

    public function getAttribute($attribute)
    {
        if (array_key_exists($attribute, $this->_attributes)) {
            return $this->_attributes[$attribute];
        } else {
            return NULL;
        }
    }

    public function setAttribute($attribute, $value)
    {
        if (array_key_exists($attribute, $this->_attributes)) {
            if (self::ATTR_ALPHABET == $attribute && !preg_match('.^[0-9a-z]{2,}$.D', $value)) { // TODO: the regexp doesn't assume characters are distinct
                return FALSE;
            } else if (self::ATTR_UNICODE_VERSION == $attribute && ($value < self::UNICODE_FIRST || $value > self::UNICODE_LAST)) {
                return FALSE;
            } else if (self::ATTR_REVERSED == $attribute && !in_array($value, array(self::ALWAYS, self::NEVER, self::RANDOM))) {
                return FALSE;
            } else if (in_array($attribute, array(self::ATTR_FAKE_CHARACTERS_COLOR, self::ATTR_SIGNIFICANT_CHARACTERS_COLOR)) && !array_key_exists($value, self::$colors)) {
                return FALSE;
            }
            $this->_attributes[$attribute] = $value;
            return TRUE;
        } else {
            return FALSE;
        }
    }
}

function captcha_create($key, CSSCaptchaStoreInterface $serializer, $options = array())
{
    return new CSSCaptcha($key, $serializer, $options);
}

function captcha_render(CSSCaptcha $captcha, $what = 0x11/*CSSCaptcha::RENDER_CSS | CSSCaptcha::RENDER_HTML*/)
{
    return $captcha->render($what);
}

function captcha_validate(CSSCaptcha $captcha, $user_input)
{
    return $captcha->validate($user_input);
}

function captcha_cleanup(CSSCaptcha $captcha)
{
    $captcha->cleanup();
}

function captcha_get_key(CSSCaptcha $captcha)
{
    return $captcha->getKey();
}

function captcha_get_challenge(CSSCaptcha $captcha)
{
    return $captcha->getChallenge();
}

function captcha_get_attribute(CSSCaptcha $captcha, $attribute)
{
    return $captcha->getAttribute($attribute);
}

function captcha_set_attribute(CSSCaptcha $captcha, $attribute, $value)
{
    return $captcha->setAttribute($attribute, $value);
}
