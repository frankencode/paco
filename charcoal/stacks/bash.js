charcoal.stacks["bash"] =
{
	title: "Bash",
	syntax: "bash",
	filename: [ "*.sh", "Makefile", "*.hxml", "*.conf" ],
	insight: {
		syntax: "bash",
		entities: {
			"BashSource": {
				category: "Void"
			},
			"FunctionDeclaration": {
				category: "Function",
				identifier: "FunctionDeclarationName"
			}
		}
	}
};
