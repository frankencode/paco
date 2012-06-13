white       = "#ffffff";
black       = "#000000";
red         = "#ff0000";
darkRed     = "#800000";
green       = "#00ff00";
darkGreen   = "#008000";
blue        = "#0000ff";
darkBlue    = "#000080";
cyan        = "#00ffff";
darkCyan    = "#008080";
magenta     = "#ff00ff";
darkMagenta = "#800080";
yellow      = "#ffff00";
darkYellow  = "#808000";
gray        = "#a0a0a4";
darkGray    = "#808080";
lightGray   = "#c0c0c0";

charcoal.themes["Default"] =
{
	displayName: "Black on White",
	
	layers: {
		"ruby": {
			"Comment": darkGray,
			"BlockComment": darkGray,
			"Keyword": { color: black, bold: true },
			"DeclarationKeyword": { color: black, bold: true },
			"Builtin": darkRed,
			"String": darkGreen,
			"EscapeSequence": darkYellow,
			"Interpolation": darkMagenta,
			"Block": black,
			"Document": darkGreen,
			"Integer": blue,
			"Float": magenta,
			"Symbol": darkCyan,
			// "LocalName": black,
			// "ParentModuleName": darkMagenta,
			"ModuleName": darkMagenta,
			"MemberName": darkMagenta,
			"ClassDeclarationName": darkMagenta,
			"MethodDeclarationName": darkBlue,
			"ClassMethodDeclarationName": darkBlue,
			"OperatorDeclarationName": darkBlue,
			// "ClassMemberName": black,
			// "ClassName": black,
			// "ConstantName": black,
			"GlobalName": darkRed // { color: black, bold: true }
		},
		"python": {
			"Comment": darkGray,
			"String": darkGreen,
			"Keyword": { color: black, bold: true },
			"Operator": black,
			"Float": magenta,
			"Integer": blue
		},
		"javascript": {
			"Comment": darkGray,
			"ReservedWord": { color: black, bold: true },
			"DecimalLiteral": blue,
			"HexIntegerLiteral": magenta,
			"StringLiteral": darkGreen,
			"EscapeSequence": darkYellow,
			"BackslashSequence": darkYellow,
			"RegularExpressionLiteral": darkMagenta,
			"BasicType": darkMagenta,
			"ClassName": darkBlue,
			"Key": darkRed
		},
		"as3": {
			"Comment": darkGray,
			"ReservedWord": { color: black, bold: true },
			"DecimalLiteral": blue,
			"HexIntegerLiteral": magenta,
			"StringLiteral": darkGreen,
			"EscapeSequence": darkYellow,
			"BackslashSequence": darkYellow,
			"RegularExpressionLiteral": darkMagenta
		},
		"haxe": {
			"Comment": darkGray,
			"String": darkGreen,
			"RegExp": darkMagenta,
			"EscapedChar": darkYellow,
			"AnyEscapedChar": darkYellow,
			"Macro": darkGreen,
			"Keyword": { color: black, bold: true },
			"PackageIdentifier": darkBlue,
			"ClassIdentifier": darkBlue,
			"FunctionIdentifier": darkBlue,
			"VariableIdentifier": darkBlue,
			"EnumIdentifier": darkBlue,
			"BuiltInIdentifier": darkRed,
			"Float": magenta,
			"Integer": blue
		},
		"tex": {
			"Comment": darkGray,
			"Command": darkBlue,
			"Number": darkMagenta,
			"Argument": blue
		},
		"css": {
			"Comment": darkGray,
			"Property": darkGreen,
			"UnknownProperty": darkRed,
			"Value": blue,
			"Unit": blue,
			"Tag": black,
			"Class": darkBlue,
			"PseudoClass": darkCyan,
			"Id": darkMagenta,
			"Rule": darkGreen
		},
		"xml": {
			"Comment": darkGray,
			"TagName": { color: black, bold: true },
			"TagOpen": { color: black, bold: true },
			"TagClose": { color: black, bold: true },
			"AttributeName": darkBlue,
			"AttributeValue": darkGreen,
			"ScriptTagName": { color: black, bold: true }
		},
		"erb": {
			"ErbComment": darkGray,
			"ErbStart": { color: darkMagenta, bold: true },
			"ErbEnd": { color: darkMagenta, bold: true }
		},
		"cucumber": {
			"Entity": { color: blue, bold: true },
			"FeatureKey": { color: blue, bold: true },
			"FeatureTitle": darkCyan,
			"FeatureClause": darkCyan,
			"ScenarioKey": { color: blue, bold: true },
			"ScenarioTitle": darkCyan,
			"ScenarioClause": black,
			"Citation": darkCyan,
			"Tag": darkMagenta,
			"Comment": darkGray
		},
		"cxx": {
			"DocCommentOpen": darkBlue,
			"DocCommentClose": darkBlue,
			"DocComment": darkGray,
			"BlockComment": darkGray,
			"LineComment": darkGray,
			"PreprocessingComment": darkGray,
			"Preprocessing": darkGreen,
			"EscapedChar": magenta,
			"String": red,
			"ObjcEscapedChar": darkYellow,
			"ObjcString": darkGreen,
			"Keyword": { color: black, bold: true },
			"TypeKeyword": darkRed,
			"QtKeyword": { color: blue, bold: true },
			"ObjcKeyword": { color: blue, bold: true },
			"Integer": blue,
			"Float": darkMagenta,
			"Char": magenta
		},
		"qdoc": {
			"Command": darkBlue,
			"OpeningOrClosingParenthesis": darkBlue,
			"Title": darkMagenta,
			"Span": darkGray,
			"VerbatimCodeBody": black
		},
		"dao": {
			"BlockComment": darkGray,
			"LineComment": darkGray,
			"EscapedChar": magenta,
			"String": red,
			"Keyword": { color: black, bold: true },
			"CoreTypes": darkRed,
			"Builtin": { color: darkBlue, bold: true },
			"Integer": blue,
			"Float": darkMagenta,
			"Char": magenta
		},
		"neko": {
			"Comment": darkGray,
			"Integer": blue,
			"Float": magenta,
			"Boolean": darkCyan,
			"String": darkGreen,
			"EscapedChar": darkYellow,
			"Builtin": darkBlue,
			// "Operator": black,
			"Keyword": { color: black, bold: true }
		},
		"bash": {
			"Comment": darkGray,
			"SingleQuoted": darkGreen,
			"DoubleQuoted": darkGreen,
			"Document": darkGreen,
			"EscapedChar": darkYellow,
			"Expansion": darkMagenta,
			// "Operator": black,
			"Builtin": darkBlue,
			"Reserved": { color: black, bold: true },
			"Integer": blue,
			"FunctionDeclarationName": darkBlue
		},
		"cmake": {
			"Comment": darkGray,
			"Command": { color: black, bold: true },
			"Function": { color: darkBlue, bold: true },
			"Expansion": darkMagenta,
			"String": darkGreen,
			"Version": blue,
			"Boolean": blue,
			"Number": blue
		},
		"sql": {
			"Comment": darkGray,
			"String": darkGreen,
			"EscapeSequence": darkYellow,
			"Number": blue,
			"Keyword": darkBlue, // { color: black, bold: true },
			"Datatype": darkRed
		},
		"pascal": {
			"Preprocessing": darkGreen,
			"Comment": darkGray,
			"String": darkGreen,
			"ControlString": darkYellow, // aka: escape sequence
			"Integer": blue,
			"Real": darkMagenta,
			"Keyword": { color: black, bold: true },
			"Modifier": darkCyan, // special type declaration keywords
			"AddressOfIdentifier": darkMagenta // color like Expansion in bash
			// "Identifier": ... // not to be highlighted, only exists for speed!
		},
		"octave": {
			"Comment": darkGray,
			"BuiltinAtomic": darkRed,
			"Builtin": darkBlue,
			"ReservedWord": { color: black, bold: true },
			"Integer": blue,
			"Float": darkMagenta,
			"SingleQuotedString": darkGreen,
			"DoubleQuotedString": darkGreen,
			"EscapeSequence": darkYellow,
			"EscapedSingleQuote": darkYellow,
			"Continuation": darkYellow,
			"FunctionHandle": darkMagenta // color like Expansion in bash
			// "Identifier": ... // not to be highlighted, only exists for speed!
		}
	},
	editor: {
		foreground: "#000",
		background: "#FFF",
		cursor: "#000",
		lineNumbers: "#808080",
		lineNumbersBright: "#000",
		lineNumbersBackground: "#F2F2F2",
		lineNumbersBackgroundBright: "#E5E5E5",
		selection: "#CCE5FF",
		match: "#FF6",
		bracketMatch: "#FFFF4C",
		currentLine: "#F2F2F2"
	}
};
