var exec = require('child_process').exec;

// var Spooky = require('spooky');
// var casper = new Spooky();
// if (false) {
var casper = require('casper').create(
    /*{
        verbose: true,
        logLevel: 'debug'
    }*/
);
// }

var require = patchRequire(require);

var map = {};

function mapTranslit(element, index, array) {
//     casper.echo(element + ' => ' + this);
    map[element] = this;
}


// <space>"(?!\\u)0? => <space>"\\u
// "\\u(...)" => "\\u0\1"
[ "\u2070", "\u2080", "\u24EA", "\uFF10", "\u1D7CE", "\u1D7D8", "\u1D7E2", "\u1D7EC", "\u1D7F6" ].forEach(mapTranslit, '0');
[ "\u00B9", "\u2081", "\u2460", "\uFF11", "\u1D7CF", "\u1D7D9", "\u1D7E3", "\u1D7ED", "\u1D7F7" ].forEach(mapTranslit, '1');
[ "\u00B2", "\u2082", "\u2461", "\uFF12", "\u1D7D0", "\u1D7DA", "\u1D7E4", "\u1D7EE", "\u1D7F8" ].forEach(mapTranslit, '2');
[ "\u00B3", "\u2083", "\u2462", "\uFF13", "\u1D7D1", "\u1D7DB", "\u1D7E5", "\u1D7EF", "\u1D7F9" ].forEach(mapTranslit, '3');
[ "\u2074", "\u2084", "\u2463", "\uFF14", "\u1D7D2", "\u1D7DC", "\u1D7E6", "\u1D7F0", "\u1D7FA" ].forEach(mapTranslit, '4');
[ "\u2075", "\u2085", "\u2464", "\uFF15", "\u1D7D3", "\u1D7DD", "\u1D7E7", "\u1D7F1", "\u1D7FB" ].forEach(mapTranslit, '5');
[ "\u2076", "\u2086", "\u2465", "\uFF16", "\u1D7D4", "\u1D7DE", "\u1D7E8", "\u1D7F2", "\u1D7FC" ].forEach(mapTranslit, '6');
[ "\u2077", "\u2087", "\u2466", "\uFF17", "\u1D7D5", "\u1D7DF", "\u1D7E9", "\u1D7F3", "\u1D7FD" ].forEach(mapTranslit, '7');
[ "\u2078", "\u2088", "\u2467", "\uFF18", "\u1D7D6", "\u1D7E0", "\u1D7EA", "\u1D7F4", "\u1D7FE" ].forEach(mapTranslit, '8');
[ "\u2079", "\u2089", "\u2468", "\uFF19", "\u1D7D7", "\u1D7E1", "\u1D7EB", "\u1D7F5", "\u1D7FF" ].forEach(mapTranslit, '9');
[ "\u00AA", "\u00C0", "\u00C1", "\u00C2", "\u00C3", "\u00C4", "\u00C5", "\u00E0", "\u00E1", "\u00E2", "\u00E3", "\u00E4", "\u00E5", "\u0100", "\u0101", "\u0102", "\u0103", "\u0104", "\u0105", "\u01CD", "\u01CE", "\u01DE", "\u01DF", "\u01E0", "\u01E1", "\u01FA", "\u01FB", "\u0200", "\u0201", "\u0202", "\u0203", "\u1E00", "\u1E01", "\u1EA0", "\u1EA1", "\u1EA2", "\u1EA3", "\u1EA4", "\u1EA5", "\u1EA6", "\u1EA7", "\u1EA8", "\u1EA9", "\u1EAA", "\u1EAB", "\u1EAC", "\u1EAD", "\u1EAE", "\u1EAF", "\u1EB0", "\u1EB1", "\u1EB2", "\u1EB3", "\u1EB4", "\u1EB5", "\u1EB6", "\u1EB7", "\u212B", "\u24B6", "\u24D0", "\uFF21", "\uFF41", "\u0226", "\u0227", "\u1D400", "\u1D41A", "\u1D434", "\u1D44E", "\u1D468", "\u1D482", "\u1D49C", "\u1D4B6", "\u1D4D0", "\u1D4EA", "\u1D504", "\u1D51E", "\u1D538", "\u1D552", "\u1D56C", "\u1D586", "\u1D5A0", "\u1D5BA", "\u1D5D4", "\u1D5EE", "\u1D608", "\u1D622", "\u1D63C", "\u1D656", "\u1D670", "\u1D68A", "\u1D2C", "\u1D43", "\u2090", "\u1F130" ].forEach(mapTranslit, 'a');
[ "\u1E02", "\u1E03", "\u1E04", "\u1E05", "\u1E06", "\u1E07", "\u212C", "\u24B7", "\u24D1", "\uFF22", "\uFF42", "\u1D401", "\u1D41B", "\u1D435", "\u1D44F", "\u1D469", "\u1D483", "\u1D4B7", "\u1D4D1", "\u1D4EB", "\u1D505", "\u1D51F", "\u1D539", "\u1D553", "\u1D56D", "\u1D587", "\u1D5A1", "\u1D5BB", "\u1D5D5", "\u1D5EF", "\u1D609", "\u1D623", "\u1D63D", "\u1D657", "\u1D671", "\u1D68B", "\u1D2E", "\u1D47", "\u1F131" ].forEach(mapTranslit, 'b');
[ "\u00C7", "\u00E7", "\u0106", "\u0107", "\u0108", "\u0109", "\u010A", "\u010B", "\u010C", "\u010D", "\u1E08", "\u1E09", "\u2102", "\u212D", "\u216D", "\u217D", "\u24B8", "\u24D2", "\uFF23", "\uFF43", "\u1D402", "\u1D41C", "\u1D436", "\u1D450", "\u1D46A", "\u1D484", "\u1D49E", "\u1D4B8", "\u1D4D2", "\u1D4EC", "\u1D520", "\u1D554", "\u1D56E", "\u1D588", "\u1D5A2", "\u1D5BC", "\u1D5D6", "\u1D5F0", "\u1D60A", "\u1D624", "\u1D63E", "\u1D658", "\u1D672", "\u1D68C", "\u1D9C", "\u1F12B", "\u1F132" ].forEach(mapTranslit, 'c');
[ "\u010E", "\u010F", "\u1E0A", "\u1E0B", "\u1E0C", "\u1E0D", "\u1E0E", "\u1E0F", "\u1E10", "\u1E11", "\u1E12", "\u1E13", "\u216E", "\u217E", "\u24B9", "\u24D3", "\uFF24", "\uFF44", "\u1D403", "\u1D41D", "\u1D437", "\u1D451", "\u1D46B", "\u1D485", "\u1D49F", "\u1D4B9", "\u1D4D3", "\u1D4ED", "\u1D507", "\u1D521", "\u1D53B", "\u1D555", "\u1D56F", "\u1D589", "\u1D5A3", "\u1D5BD", "\u1D5D7", "\u1D5F1", "\u1D60B", "\u1D625", "\u1D63F", "\u1D659", "\u1D673", "\u1D68D", "\u2145", "\u2146", "\u1D30", "\u1D48", "\u1F133" ].forEach(mapTranslit, 'd');
[ "\u00C8", "\u00C9", "\u00CA", "\u00CB", "\u00E8", "\u00E9", "\u00EA", "\u00EB", "\u0112", "\u0113", "\u0114", "\u0115", "\u0116", "\u0117", "\u0118", "\u0119", "\u011A", "\u011B", "\u0204", "\u0205", "\u0206", "\u0207", "\u1E14", "\u1E15", "\u1E16", "\u1E17", "\u1E18", "\u1E19", "\u1E1A", "\u1E1B", "\u1E1C", "\u1E1D", "\u1EB8", "\u1EB9", "\u1EBA", "\u1EBB", "\u1EBC", "\u1EBD", "\u1EBE", "\u1EBF", "\u1EC0", "\u1EC1", "\u1EC2", "\u1EC3", "\u1EC4", "\u1EC5", "\u1EC6", "\u1EC7", "\u212F", "\u2130", "\u24BA", "\u24D4", "\uFF25", "\uFF45", "\u0228", "\u0229", "\u1D404", "\u1D41E", "\u1D438", "\u1D452", "\u1D46C", "\u1D486", "\u1D4D4", "\u1D4EE", "\u1D508", "\u1D522", "\u1D53C", "\u1D556", "\u1D570", "\u1D58A", "\u1D5A4", "\u1D5BE", "\u1D5D8", "\u1D5F2", "\u1D60C", "\u1D626", "\u1D640", "\u1D65A", "\u1D674", "\u1D68E", "\u2147", "\u1D31", "\u1D49", "\u2091", "\u1F134" ].forEach(mapTranslit, 'e');
[ "\u1E1E", "\u1E1F", "\u2131", "\u24BB", "\u24D5", "\uFF26", "\uFF46", "\u1D405", "\u1D41F", "\u1D439", "\u1D453", "\u1D46D", "\u1D487", "\u1D4BB", "\u1D4D5", "\u1D4EF", "\u1D509", "\u1D523", "\u1D53D", "\u1D557", "\u1D571", "\u1D58B", "\u1D5A5", "\u1D5BF", "\u1D5D9", "\u1D5F3", "\u1D60D", "\u1D627", "\u1D641", "\u1D65B", "\u1D675", "\u1D68F", "\u1DA0", "\u1F135" ].forEach(mapTranslit, 'f');
[ "\u011C", "\u011D", "\u011E", "\u011F", "\u0120", "\u0121", "\u0122", "\u0123", "\u01E6", "\u01E7", "\u01F4", "\u01F5", "\u1E20", "\u1E21", "\u210A", "\u24BC", "\u24D6", "\uFF27", "\uFF47", "\u1D406", "\u1D420", "\u1D43A", "\u1D454", "\u1D46E", "\u1D488", "\u1D4A2", "\u1D4D6", "\u1D4F0", "\u1D50A", "\u1D524", "\u1D53E", "\u1D558", "\u1D572", "\u1D58C", "\u1D5A6", "\u1D5C0", "\u1D5DA", "\u1D5F4", "\u1D60E", "\u1D628", "\u1D642", "\u1D65C", "\u1D676", "\u1D690", "\u1D33", "\u1D4D", "\u1F136" ].forEach(mapTranslit, 'g');
[ "\u0124", "\u0125", "\u02B0", "\u1E22", "\u1E23", "\u1E24", "\u1E25", "\u1E26", "\u1E27", "\u1E28", "\u1E29", "\u1E2A", "\u1E2B", "\u1E96", "\u210B", "\u210C", "\u210D", "\u210E", "\u24BD", "\u24D7", "\uFF28", "\uFF48", "\u021E", "\u021F", "\u1D407", "\u1D421", "\u1D43B", "\u1D46F", "\u1D489", "\u1D4BD", "\u1D4D7", "\u1D4F1", "\u1D525", "\u1D559", "\u1D573", "\u1D58D", "\u1D5A7", "\u1D5C1", "\u1D5DB", "\u1D5F5", "\u1D60F", "\u1D629", "\u1D643", "\u1D65D", "\u1D677", "\u1D691", "\u1D34", "\u2095", "\u1F137" ].forEach(mapTranslit, 'h');
[ "\u00CC", "\u00CD", "\u00CE", "\u00CF", "\u00EC", "\u00ED", "\u00EE", "\u00EF", "\u0128", "\u0129", "\u012A", "\u012B", "\u012C", "\u012D", "\u012E", "\u012F", "\u0130", "\u01CF", "\u01D0", "\u0208", "\u0209", "\u020A", "\u020B", "\u1E2C", "\u1E2D", "\u1E2E", "\u1E2F", "\u1EC8", "\u1EC9", "\u1ECA", "\u1ECB", "\u2110", "\u2111", "\u2160", "\u2170", "\u24BE", "\u24D8", "\uFF29", "\uFF49", "\u2139", "\u1D408", "\u1D422", "\u1D43C", "\u1D456", "\u1D470", "\u1D48A", "\u1D4BE", "\u1D4D8", "\u1D4F2", "\u1D526", "\u1D540", "\u1D55A", "\u1D574", "\u1D58E", "\u1D5A8", "\u1D5C2", "\u1D5DC", "\u1D5F6", "\u1D610", "\u1D62A", "\u1D644", "\u1D65E", "\u1D678", "\u1D692", "\u2071", "\u2148", "\u1D35", "\u1D62", "\u1F138" ].forEach(mapTranslit, 'i');
[ "\u0134", "\u0135", "\u01F0", "\u02B2", "\u24BF", "\u24D9", "\uFF2A", "\uFF4A", "\u1D409", "\u1D423", "\u1D43D", "\u1D457", "\u1D471", "\u1D48B", "\u1D4A5", "\u1D4BF", "\u1D4D9", "\u1D4F3", "\u1D50D", "\u1D527", "\u1D541", "\u1D55B", "\u1D575", "\u1D58F", "\u1D5A9", "\u1D5C3", "\u1D5DD", "\u1D5F7", "\u1D611", "\u1D62B", "\u1D645", "\u1D65F", "\u1D679", "\u1D693", "\u2149", "\u1D36", "\u2C7C", "\u1F139" ].forEach(mapTranslit, 'j');
[ "\u0136", "\u0137", "\u01E8", "\u01E9", "\u1E30", "\u1E31", "\u1E32", "\u1E33", "\u1E34", "\u1E35", "\u212A", "\u24C0", "\u24DA", "\uFF2B", "\uFF4B", "\u1D40A", "\u1D424", "\u1D43E", "\u1D458", "\u1D472", "\u1D48C", "\u1D4A6", "\u1D4C0", "\u1D4DA", "\u1D4F4", "\u1D50E", "\u1D528", "\u1D542", "\u1D55C", "\u1D576", "\u1D590", "\u1D5AA", "\u1D5C4", "\u1D5DE", "\u1D5F8", "\u1D612", "\u1D62C", "\u1D646", "\u1D660", "\u1D67A", "\u1D694", "\u1D37", "\u1D4F", "\u2096", "\u1F13A" ].forEach(mapTranslit, 'k');
[ "\u0139", "\u013A", "\u013B", "\u013C", "\u013D", "\u013E", "\u02E1", "\u1E36", "\u1E37", "\u1E38", "\u1E39", "\u1E3A", "\u1E3B", "\u1E3C", "\u1E3D", "\u2112", "\u2113", "\u216C", "\u217C", "\u24C1", "\u24DB", "\uFF2C", "\uFF4C", "\u1D40B", "\u1D425", "\u1D43F", "\u1D459", "\u1D473", "\u1D48D", "\u1D4DB", "\u1D4F5", "\u1D50F", "\u1D529", "\u1D543", "\u1D55D", "\u1D577", "\u1D591", "\u1D5AB", "\u1D5C5", "\u1D5DF", "\u1D5F9", "\u1D613", "\u1D62D", "\u1D647", "\u1D661", "\u1D67B", "\u1D695", "\u1D38", "\u1D4C1", "\u2097", "\u1F13B" ].forEach(mapTranslit, 'l');
[ "\u1E3E", "\u1E3F", "\u1E40", "\u1E41", "\u1E42", "\u1E43", "\u2133", "\u216F", "\u217F", "\u24C2", "\u24DC", "\uFF2D", "\uFF4D", "\u1D40C", "\u1D426", "\u1D440", "\u1D45A", "\u1D474", "\u1D48E", "\u1D4C2", "\u1D4DC", "\u1D4F6", "\u1D510", "\u1D52A", "\u1D544", "\u1D55E", "\u1D578", "\u1D592", "\u1D5AC", "\u1D5C6", "\u1D5E0", "\u1D5FA", "\u1D614", "\u1D62E", "\u1D648", "\u1D662", "\u1D67C", "\u1D696", "\u1D39", "\u1D50", "\u2098", "\u1F13C" ].forEach(mapTranslit, 'm');
[ "\u00D1", "\u00F1", "\u0143", "\u0144", "\u0145", "\u0146", "\u0147", "\u0148", "\u1E44", "\u1E45", "\u1E46", "\u1E47", "\u1E48", "\u1E49", "\u1E4A", "\u1E4B", "\u207F", "\u2115", "\u24C3", "\u24DD", "\uFF2E", "\uFF4E", "\u01F8", "\u01F9", "\u1D40D", "\u1D427", "\u1D441", "\u1D45B", "\u1D475", "\u1D48F", "\u1D4A9", "\u1D4C3", "\u1D4DD", "\u1D4F7", "\u1D511", "\u1D52B", "\u1D55F", "\u1D579", "\u1D593", "\u1D5AD", "\u1D5C7", "\u1D5E1", "\u1D5FB", "\u1D615", "\u1D62F", "\u1D649", "\u1D663", "\u1D67D", "\u1D697", "\u1D3A", "\u1F13D", "\u2099" ].forEach(mapTranslit, 'n');
[ "\u00BA", "\u00D2", "\u00D3", "\u00D4", "\u00D5", "\u00D6", "\u00F2", "\u00F3", "\u00F4", "\u00F5", "\u00F6", "\u014C", "\u014D", "\u014E", "\u014F", "\u0150", "\u0151", "\u01A0", "\u01A1", "\u01D1", "\u01D2", "\u01EA", "\u01EB", "\u01EC", "\u01ED", "\u020C", "\u020D", "\u020E", "\u020F", "\u1E4C", "\u1E4D", "\u1E4E", "\u1E4F", "\u1E50", "\u1E51", "\u1E52", "\u1E53", "\u1ECC", "\u1ECD", "\u1ECE", "\u1ECF", "\u1ED0", "\u1ED1", "\u1ED2", "\u1ED3", "\u1ED4", "\u1ED5", "\u1ED6", "\u1ED7", "\u1ED8", "\u1ED9", "\u1EDA", "\u1EDB", "\u1EDC", "\u1EDD", "\u1EDE", "\u1EDF", "\u1EE0", "\u1EE1", "\u1EE2", "\u1EE3", "\u2134", "\u24C4", "\u24DE", "\uFF2F", "\uFF4F", "\u022A", "\u022B", "\u022C", "\u022D", "\u022E", "\u022F", "\u0230", "\u0231", "\u1D40E", "\u1D428", "\u1D442", "\u1D45C", "\u1D476", "\u1D490", "\u1D4AA", "\u1D4DE", "\u1D4F8", "\u1D512", "\u1D52C", "\u1D546", "\u1D560", "\u1D57A", "\u1D594", "\u1D5AE", "\u1D5C8", "\u1D5E2", "\u1D5FC", "\u1D616", "\u1D630", "\u1D64A", "\u1D664", "\u1D67E", "\u1D698", "\u1D3C", "\u1D52", "\u2092", "\u1F13E" ].forEach(mapTranslit, 'o');
[ "\u1E54", "\u1E55", "\u1E56", "\u1E57", "\u2119", "\u24C5", "\u24DF", "\uFF30", "\uFF50", "\u1D40F", "\u1D429", "\u1D443", "\u1D45D", "\u1D477", "\u1D491", "\u1D4AB", "\u1D4C5", "\u1D4DF", "\u1D4F9", "\u1D513", "\u1D52D", "\u1D561", "\u1D57B", "\u1D595", "\u1D5AF", "\u1D5C9", "\u1D5E3", "\u1D5FD", "\u1D617", "\u1D631", "\u1D64B", "\u1D665", "\u1D67F", "\u1D699", "\u1D3E", "\u1D56", "\u1F13F", "\u209A" ].forEach(mapTranslit, 'p');
[ "\u211A", "\u24C6", "\u24E0", "\uFF31", "\uFF51", "\u1D410", "\u1D42A", "\u1D444", "\u1D45E", "\u1D478", "\u1D492", "\u1D4AC", "\u1D4C6", "\u1D4E0", "\u1D4FA", "\u1D514", "\u1D52E", "\u1D562", "\u1D57C", "\u1D596", "\u1D5B0", "\u1D5CA", "\u1D5E4", "\u1D5FE", "\u1D618", "\u1D632", "\u1D64C", "\u1D666", "\u1D680", "\u1D69A", "\u1F140" ].forEach(mapTranslit, 'q');
[ "\u0154", "\u0155", "\u0156", "\u0157", "\u0158", "\u0159", "\u0210", "\u0211", "\u0212", "\u0213", "\u02B3", "\u1E58", "\u1E59", "\u1E5A", "\u1E5B", "\u1E5C", "\u1E5D", "\u1E5E", "\u1E5F", "\u211B", "\u211C", "\u211D", "\u24C7", "\u24E1", "\uFF32", "\uFF52", "\u1D411", "\u1D42B", "\u1D445", "\u1D45F", "\u1D479", "\u1D493", "\u1D4C7", "\u1D4E1", "\u1D4FB", "\u1D52F", "\u1D563", "\u1D57D", "\u1D597", "\u1D5B1", "\u1D5CB", "\u1D5E5", "\u1D5FF", "\u1D619", "\u1D633", "\u1D64D", "\u1D667", "\u1D681", "\u1D69B", "\u1D3F", "\u1D63", "\u1F12C", "\u1F141" ].forEach(mapTranslit, 'r');
[ "\u015A", "\u015B", "\u015C", "\u015D", "\u015E", "\u015F", "\u0160", "\u0161", "\u017F", "\u02E2", "\u1E60", "\u1E61", "\u1E62", "\u1E63", "\u1E64", "\u1E65", "\u1E66", "\u1E67", "\u1E68", "\u1E69", "\u24C8", "\u24E2", "\uFF33", "\uFF53", "\u1E9B", "\u0218", "\u0219", "\u1D412", "\u1D42C", "\u1D446", "\u1D460", "\u1D47A", "\u1D494", "\u1D4AE", "\u1D4C8", "\u1D4E2", "\u1D4FC", "\u1D516", "\u1D530", "\u1D54A", "\u1D564", "\u1D57E", "\u1D598", "\u1D5B2", "\u1D5CC", "\u1D5E6", "\u1D600", "\u1D61A", "\u1D634", "\u1D64E", "\u1D668", "\u1D682", "\u1D69C", "\u1F142", "\u209B" ].forEach(mapTranslit, 's');
[ "\u0162", "\u0163", "\u0164", "\u0165", "\u1E6A", "\u1E6B", "\u1E6C", "\u1E6D", "\u1E6E", "\u1E6F", "\u1E70", "\u1E71", "\u1E97", "\u24C9", "\u24E3", "\uFF34", "\uFF54", "\u021A", "\u021B", "\u1D413", "\u1D42D", "\u1D447", "\u1D461", "\u1D47B", "\u1D495", "\u1D4AF", "\u1D4C9", "\u1D4E3", "\u1D4FD", "\u1D517", "\u1D531", "\u1D54B", "\u1D565", "\u1D57F", "\u1D599", "\u1D5B3", "\u1D5CD", "\u1D5E7", "\u1D601", "\u1D61B", "\u1D635", "\u1D64F", "\u1D669", "\u1D683", "\u1D69D", "\u1D40", "\u1D57", "\u209C", "\u1F143" ].forEach(mapTranslit, 't');
[ "\u00D9", "\u00DA", "\u00DB", "\u00DC", "\u00F9", "\u00FA", "\u00FB", "\u00FC", "\u0168", "\u0169", "\u016A", "\u016B", "\u016C", "\u016D", "\u016E", "\u016F", "\u0170", "\u0171", "\u0172", "\u0173", "\u01AF", "\u01B0", "\u01D3", "\u01D4", "\u01D5", "\u01D6", "\u01D7", "\u01D8", "\u01D9", "\u01DA", "\u01DB", "\u01DC", "\u0214", "\u0215", "\u0216", "\u0217", "\u1E72", "\u1E73", "\u1E74", "\u1E75", "\u1E76", "\u1E77", "\u1E78", "\u1E79", "\u1E7A", "\u1E7B", "\u1EE4", "\u1EE5", "\u1EE6", "\u1EE7", "\u1EE8", "\u1EE9", "\u1EEA", "\u1EEB", "\u1EEC", "\u1EED", "\u1EEE", "\u1EEF", "\u1EF0", "\u1EF1", "\u24CA", "\u24E4", "\uFF35", "\uFF55", "\u1D414", "\u1D42E", "\u1D448", "\u1D462", "\u1D47C", "\u1D496", "\u1D4B0", "\u1D4CA", "\u1D4E4", "\u1D4FE", "\u1D518", "\u1D532", "\u1D54C", "\u1D566", "\u1D580", "\u1D59A", "\u1D5B4", "\u1D5CE", "\u1D5E8", "\u1D602", "\u1D61C", "\u1D636", "\u1D650", "\u1D66A", "\u1D684", "\u1D69E", "\u1D41", "\u1D58", "\u1D64", "\u1F144" ].forEach(mapTranslit, 'u');
[ "\u1E7C", "\u1E7D", "\u1E7E", "\u1E7F", "\u2164", "\u2174", "\u24CB", "\u24E5", "\uFF36", "\uFF56", "\u1D415", "\u1D42F", "\u1D449", "\u1D463", "\u1D47D", "\u1D497", "\u1D4B1", "\u1D4CB", "\u1D4E5", "\u1D4FF", "\u1D519", "\u1D533", "\u1D54D", "\u1D567", "\u1D581", "\u1D59B", "\u1D5B5", "\u1D5CF", "\u1D5E9", "\u1D603", "\u1D61D", "\u1D637", "\u1D651", "\u1D66B", "\u1D685", "\u1D69F", "\u1D5B", "\u1D65", "\u2C7D", "\u1F145" ].forEach(mapTranslit, 'v');
[ "\u0174", "\u0175", "\u02B7", "\u1E80", "\u1E81", "\u1E82", "\u1E83", "\u1E84", "\u1E85", "\u1E86", "\u1E87", "\u1E88", "\u1E89", "\u1E98", "\u24CC", "\u24E6", "\uFF37", "\uFF57", "\u1D416", "\u1D430", "\u1D44A", "\u1D464", "\u1D47E", "\u1D498", "\u1D4B2", "\u1D4CC", "\u1D4E6", "\u1D500", "\u1D51A", "\u1D534", "\u1D54E", "\u1D568", "\u1D582", "\u1D59C", "\u1D5B6", "\u1D5D0", "\u1D5EA", "\u1D604", "\u1D61E", "\u1D638", "\u1D652", "\u1D66C", "\u1D686", "\u1D6A0", "\u1D42", "\u1F146" ].forEach(mapTranslit, 'w');
[ "\u02E3", "\u1E8A", "\u1E8B", "\u1E8C", "\u1E8D", "\u2169", "\u2179", "\u24CD", "\u24E7", "\uFF38", "\uFF58", "\u1D417", "\u1D431", "\u1D44B", "\u1D465", "\u1D47F", "\u1D499", "\u1D4B3", "\u1D4CD", "\u1D4E7", "\u1D501", "\u1D51B", "\u1D535", "\u1D54F", "\u1D569", "\u1D583", "\u1D59D", "\u1D5B7", "\u1D5D1", "\u1D5EB", "\u1D605", "\u1D61F", "\u1D639", "\u1D653", "\u1D66D", "\u1D687", "\u1D6A1", "\u2093", "\u1F147" ].forEach(mapTranslit, 'x');
[ "\u00DD", "\u00FD", "\u00FF", "\u0176", "\u0177", "\u0178", "\u02B8", "\u1E8E", "\u1E8F", "\u1E99", "\u1EF2", "\u1EF3", "\u1EF4", "\u1EF5", "\u1EF6", "\u1EF7", "\u1EF8", "\u1EF9", "\u24CE", "\u24E8", "\uFF39", "\uFF59", "\u0232", "\u0233", "\u1D418", "\u1D432", "\u1D44C", "\u1D466", "\u1D480", "\u1D49A", "\u1D4B4", "\u1D4CE", "\u1D4E8", "\u1D502", "\u1D51C", "\u1D536", "\u1D550", "\u1D56A", "\u1D584", "\u1D59E", "\u1D5B8", "\u1D5D2", "\u1D5EC", "\u1D606", "\u1D620", "\u1D63A", "\u1D654", "\u1D66E", "\u1D688", "\u1D6A2", "\u1F148" ].forEach(mapTranslit, 'y');
[ "\u0179", "\u017A", "\u017B", "\u017C", "\u017D", "\u017E", "\u1E90", "\u1E91", "\u1E92", "\u1E93", "\u1E94", "\u1E95", "\u2124", "\u2128", "\u24CF", "\u24E9", "\uFF3A", "\uFF5A", "\u1D419", "\u1D433", "\u1D44D", "\u1D467", "\u1D481", "\u1D49B", "\u1D4B5", "\u1D4CF", "\u1D4E9", "\u1D503", "\u1D537", "\u1D56B", "\u1D585", "\u1D59F", "\u1D5B9", "\u1D5D3", "\u1D5ED", "\u1D607", "\u1D621", "\u1D63B", "\u1D655", "\u1D66F", "\u1D689", "\u1D6A3", "\u1DBB", "\u1F149" ].forEach(mapTranslit, 'z');

/*
A problem seems to be that codepoints which can't be displayed with the current font are converted to U+FFFD so we can't "decipher" them
*/
casper.start(
    'http://localhost:3000/hidden_fakes.php',
    function () {
        var token = this.evaluate(
            function () {
                var ignorables = [ "\u0009", "\u000A", "\u000C", "\u000D", "\u0020", "\u00A0", "\u1680", "\u180E", "\u2000", "\u2001", "\u2002", "\u2003", "\u2004", "\u2005", "\u2006", "\u2007", "\u2008", "\u2009", "\u200A", "\u2028", "\u2029", "\u202F", "\u205F", "\u3000" ];
                var captcha = document.getElementById('captcha');
                var s = [];
                var re = new RegExp('[' +  ignorables.join('') + ']', "g");
                var l = document.querySelectorAll('#captcha > span');
                for (var i = 0; i < l.length; i++) {
                    var style = getComputedStyle(l[i], ':after');
                    if (style.display != 'none') {
                        s.push(style.content.replace(re, ''));
                    }
                }

                return s;
            }
        );
//        this.echo('Token : ' + token.join('-'));
        for (var i = 0; i < token.length; i++) {
/*
this.echo(i + ' : ' + token[i]);
for (var j = 0; j < token[i].length; j++) {
this.echo('    ' + j + ' : ' + token[i].charCodeAt(j));
}
*/
//            this.echo(token[i] + ' <=> ' + map[token[i]]);
            token[i] = map[token[i]];
        }
        token = token.join('');
        this.echo('Token is : ' + token);
    }
);

casper.run();
