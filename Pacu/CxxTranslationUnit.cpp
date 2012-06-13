#include "CxxLexConvention.hpp"
#include "CxxTranslationUnit.hpp"

namespace pacu
{

CxxTranslationUnit::CxxTranslationUnit()
{
	//  -------------------- Fundamentals ----------------------------
	
	DEFINE("identifier",
		CHAR(identifierToken)
	);
	
	DEFINE("integer",
		CHAR(integerToken)
	);
	
	DEFINE("curlyBlock",
		CHAR(openingCurlyKeyword,
			REPEAT(0, intMax,
				OR(
					REF("curlyBlock"),
					OTHER(closingCurlyKeyword)
				),
				CHAR(closingCurlyKeyword)
			)
		)
	);
	
	DEFINE("atomicName",
		OR(
			// primary
			OR(
				OR(
					CHAR(intKeyword),
					CHAR(boolKeyword)
				),
				OR(
					OR(
						CHAR(floatKeyword),
						CHAR(doubleKeyword)
					),
					OR(
						CHAR(charKeyword),
						CHAR(wcharKeyword)
					)
				)
			),
			//secondary
			OR(
				// combined
				OR(
					OR(
						CHAR(unsignedKeyword),
						CHAR(signedKeyword),
						REPEAT(0, 1,
							OR(
								CHAR(intKeyword),
								CHAR(charKeyword)
							)
						)
					),
					OR(
						CHAR(longKeyword),
						CHAR(shortKeyword),
						REPEAT(0, 1,
							OR(
								CHAR(intKeyword),
								OR(
									CHAR(floatKeyword),
									CHAR(doubleKeyword)
								)
							)
						)
					)
				),
				// detailed
				OR(
					CHAR(uint8Keyword),
					OR(
						CHAR(uint16Keyword),
						OR(
							CHAR(uint32Keyword),
							OR(
								CHAR(uint64Keyword),
								OR(
									CHAR(int8Keyword),
									OR(
										CHAR(int16Keyword),
										OR(
											CHAR(int32Keyword),
											OR(
												CHAR(int64Keyword),
												OR(
													CHAR(float32Keyword),
													CHAR(float64Keyword)
												)
											)
										)
									)
								)
							)
						)
					)
				)
			)
		)
	);
	
	// --------------- Classes (A.8, A.9) ---------------------------
	
	DEFINE("classSpecifier",
		REF("classHead",
			REF("classBody")
		)
	);
	
	DEFINE("classHead",
		REF("classKey",
			REPEAT(0, 1,
				REF("identifier"),
				REPEAT(0, 1,
					REF("baseClause")
				)
			)
		)
	);
	
	DEFINE("classBody",
		CHAR(openingCurlyKeyword,
			REPEAT(0, intMax,
				OR(
					OR(
						REF("classSectionHeader"),
						REF("curlyBlock")
					),
					OR(
						OR(
							OR(
								REF("functionSpecifier"),
								OR(
									REF("objectSpecifier"),
									OR(
										REF("enumDefinition"),
										REF("typedefDefinition")
									)
								)
							),
							OR(
								REF("destructorSpecifier"),
								REF("constructorSpecifier")
							)
						),
						OR(
							REF("block"),
							OTHER(closingCurlyKeyword)
						)
					)
				),
				CHAR(closingCurlyKeyword)
			)
		)
	);
	
	DEFINE("classKey",
		OR(
			CHAR(classKeyword),
			OR(
				CHAR(structKeyword),
				CHAR(unionKeyword)
			)
		)
	);
	
	DEFINE("baseClause",
		CHAR(colonKeyword,
			REF("baseSpecifier",
				REPEAT(0, intMax,
					CHAR(commaKeyword,
						REF("baseSpecifier")
					)
				)
			)
		)
	);
	
	DEFINE("baseSpecifier",
		OR(
			REPEAT(0, 1,
				CHAR(doubleColonKeyword),
				REF("nestedNameSpecifier",
					REF("className")
				)
			)
			,
			OR(
				CHAR(virtualKeyword,
					REPEAT(0, 1,
						REF("accessSpecifier"),
						REPEAT(0, 1,
							CHAR(doubleColonKeyword),
							REPEAT(0, 1,
								REF("nestedNameSpecifier"),
								REF("className")
							)
						)
					)
				),
				REF("accessSpecifier",
					REPEAT(0, 1,
						CHAR(virtualKeyword),
						REPEAT(0, 1,
							CHAR(doubleColonKeyword),
							REPEAT(0, 1,
								REF("nestedNameSpecifier"),
								REF("className")
							)
						)
					)
				)
			)
		)
	);
	
	DEFINE("accessSpecifier",
		OR(
			CHAR(publicKeyword),
			OR(
				CHAR(protectedKeyword),
				CHAR(privateKeyword)
			)
		)
	);
	
	DEFINE("nestedNameSpecifier", 
		REPEAT(1, intMax,
			REF("className",
				CHAR(doubleColonKeyword)
			)
		)
	);
	
	DEFINE("className",
		OR(
			REF("templateName"),
			REF("identifier")
		)
	);
	
	DEFINE("classSectionHeader",
		OR(
			OR(
				CHAR(publicKeyword),
				CHAR(qtSignalsKeyword)
			),
			OR(
				CHAR(privateKeyword),
				CHAR(protectedKeyword)
			),
			REPEAT(0, 1,
				CHAR(qtSlotsKeyword),
				CHAR(colonKeyword)
			)
		)
	);
	
	// ----------------- Namespaces (A.6) ---------------------------
	
	DEFINE("namespaceDefinition",
		CHAR(namespaceKeyword,
			REF("identifier",
				REF("namespaceBody")
			)
		)
	);
	
	DEFINE("namespaceBody",
		CHAR(openingCurlyKeyword,
			REPEAT(0, intMax,
				OR(
					REF("curlyBlock"),
					OR(
						OR(
							REF("functionSpecifier"),
							REF("objectSpecifier")
						),
						OR(
							REF("block"),
							OTHER(closingCurlyKeyword)
						)
					)
				),
				CHAR(closingCurlyKeyword)
			)
		)
	);
	
	// ----------------- Templates (A.12) ---------------------------
	
	DEFINE("templateDeclaration",
		REPEAT(1, intMax,
			CHAR(templateKeyword,
				REF("templateParameterList")
			),
			OR(
				REF("classSpecifier"),
				OR(
					REF("functionSpecifier"),
					REF("constructorSpecifier")
				)
			)
		)
	);
	
	DEFINE("templateParameterList",
		CHAR(lowerKeyword,
			REPEAT(0, intMax,
				OR(
					REF("templateArgumentBlock"),
					OTHER(greaterKeyword)
				),
				CHAR(greaterKeyword)
			)
		)
	);
	
	DEFINE("templateArgumentBlock",
		CHAR(lowerKeyword,
			REPEAT(0, intMax,
				OR(
					REF("templateArgumentBlock"),
					OTHER(greaterKeyword)
				),
				CHAR(greaterKeyword)
			)
		)
	);
	
	DEFINE("templateName",
		REF("identifier",
			REF("templateArgumentBlock")
		)
	);
	
	// ---------------------- Objects -------------------------------
	
	DEFINE("typeName",
		OR(
			OR(
				CHAR(voidKeyword),
				REF("atomicName")
			),
			REPEAT(0, 1,
				CHAR(typenameKeyword),
				REF("className")
			)
		)
	);
	
	DEFINE("constVolatileQualifier",
		OR(
			CHAR(constKeyword,
				REPEAT(0, 1,
					CHAR(volatileKeyword)
				)
			),
			CHAR(volatileKeyword,
				REPEAT(0, 1,
					CHAR(constKeyword)
				)
			)
		)
	);
	
	DEFINE("pointerQualifier",
		OR(
			REPEAT(1, intMax,
				CHAR(starKeyword)
			),
			CHAR(ampersandKeyword)
		)
	);
	
	DEFINE("arrayQualifier",
		REPEAT(1, intMax,
			CHAR(openingSquaredKeyword,
				CHAR(integerToken,
					CHAR(closingSquaredKeyword)
				)
			)
		)
	);
	
	DEFINE("objectDefaultList",
		CHAR(openingCurlyKeyword,
			REPEAT(0, intMax,
				OR(
					REF("objectDefaultList"),
					OTHER(closingCurlyKeyword)
				),
				CHAR(closingCurlyKeyword)
			)
		)
	);
	
	DEFINE("subExpression",
		CHAR(openingRoundKeyword,
			REPEAT(0, intMax,
				OR(
					REF("subExpression"),
					OTHER(closingRoundKeyword)
				),
				CHAR(closingRoundKeyword)
			)
		)
	);
		
	DEFINE("objectDefaultValue",
		REPEAT(1, intMax,
			OR(
				REF("subExpression"),
				NOT(
					OR(
						CHAR(commaKeyword),
						OR(
							CHAR(semicolonKeyword),
							OR(
								CHAR(closingRoundKeyword),
								CHAR(closingCurlyKeyword)
							)
						)
					),
					ANY()
				)
			)
		)
	);
	
	DEFINE("objectInitializer",
		CHAR(equalsKeyword,
			OR(
				REF("objectDefaultList"),
				REF("objectDefaultValue")
			)
		)
	);
	
	DEFINE("objectName",
		OR(
			REF("identifier"),
			REF("operatorName")
		)
	);
	
	DEFINE("operatorName",
		CHAR(operatorKeyword,
			OR(
				OR(
					CHAR(openingRoundKeyword,
						CHAR(closingRoundKeyword)
					),
					CHAR(openingSquaredKeyword,
						CHAR(closingSquaredKeyword)
					)
				),
				RANGE(firstOperator, lastOperator)
			)
		)
	);
	
	DEFINE("castOperatorSpecifier",
		CHAR(operatorKeyword,
			REPEAT(0, 1,
				REF("constVolatileQualifier"),
				REF("typeName",
					REPEAT(0, 1,
						REF("pointerQualifier")
					)
				)
			)
		)
	);
	
	DEFINE("objectSpecifier",
		REPEAT(0, 1,
			REF("constVolatileQualifier"),
			REF("typeName",
				REPEAT(0, 1,
					pointerQualifier_,
					REPEAT(0, 1,
						REF("nestedNameSpecifier"),
						REF("objectName",
							REPEAT(0, 1,
								REF("arrayQualifier"),
								REPEAT(0, 1,
									REF("objectInitializer")
								)
							)
						)
					)
				)
			)
		)
	);
	
	// ------------------ Functions ---------------------------------
	
	DEFINE("functionQualifier",
		OR(
			CHAR(inlineKeyword,
				REPEAT(0, 1,
					OR(
						CHAR(virtualKeyword),
						CHAR(staticKeyword)
					)
				)
			),
			OR(
				CHAR(virtualKeyword),
				CHAR(staticKeyword),
				REPEAT(0, 1,
					CHAR(inlineKeyword)
				)
			)
		)
	);
	
	DEFINE("abstractQualifier",
		CHAR(equalsKeyword,
			CHAR(integerToken)
		)
	);
	
	DEFINE("functionParameterList",
		CHAR(openingRoundKeyword,
			REPEAT(0, intMax,
				REF("objectSpecifier",
					REPEAT(0, 1,
						CHAR(commaKeyword)
					)
				),
				CHAR(closingRoundKeyword)
			)
		)
	);
	
	DEFINE("functionSpecifier",
		REPEAT(0, 1,
			REF("functionQualifier"),
			OR(
				REF("objectSpecifier"),
				REF("castOperatorSpecifier"),
				REF("functionParameterList",
					REPEAT(0, 1,
						REF("constVolatileQualifier"),
						OR(
							REF("abstractQualifier"),
							REPEAT(0, 1,
								REF("curlyBlock")
							)
						)
					)
				)
			)
		)
	);
	
	DEFINE("explicitQualifier",
		CHAR(explicitKeyword)
	);
	
	DEFINE("constructorSpecifier",
		REPEAT(0, 1,
			REF("explicitQualifier"),
			REF("identifier",
				REF("functionParameterList",
					REPEAT(0, 1,
						CHAR(colonKeyword,
							REPEAT(0, intMax,
								REF("identifier",
									REF("subExpression",
										REPEAT(0, 1,
											CHAR(commaKeyword)
										)
									)
								)
							)
						),
						REPEAT(0, 1,
							REF("curlyBlock")
						)
					)
				)
			)
		)
	);
	
	DEFINE("destructorSpecifier",
		REPEAT(0, 1,
			REF("functionQualifier"),
			CHAR(tildeKeyword,
				REF("identifier",
					REF("functionParameterList",
						REPEAT(0, 1,
							REF("curlyBlock")
						)
					)
				)
			)
		)
	);
	
	// --------------- Enumerations and Type Alias -------------------
	
	DEFINE("enumDefinition",
		CHAR(enumKeyword,
			REPEAT(0, 1,
				REF("identifier"),
				CHAR(openingCurlyKeyword,
					REPEAT(0, intMax,
						REF("identifier",
							REPEAT(0, 1,
								CHAR(equalsKeyword,
									REF("integer")
								),
								REPEAT(0, 1,
									CHAR(commaKeyword)
								)
							)
						),
						CHAR(closingCurlyKeyword)
					)
				)
			)
		)
	);
	
	DEFINE("typedefDefinition",
		CHAR(typedefKeyword,
			REF("objectSpecifier")
		)
	);
	
	// ---------------- Global Parser Logic -------------------------
	
	DEFINE("block",
		OR(
			OR(
				REF("namespaceDefinition"),
				OR(
					REF("enumDefinition"),
					REF("typedefDefinition")
				)
			),
			OR(
				REF("templateDeclaration"),
				REF("classSpecifier")
			)
		)
	);
	
	DEFINE_SELF("translationUnit",
		REPEAT(0, intMax,
			FIND(
				REF("block")
			)
		)
	);
}

Ref<Token, Owner> CxxTranslationUnit::match(Array<int>* media)
{
	
	Ref<Token, Owner> rootToken = 0;
	int i0 = 0, i1 = 0;
	SyntaxRule< Array<int> >::match(media, i0, &i1, &rootToken);
	
	return rootToken;
}

} // namespace pacu
