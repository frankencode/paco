charcoal.stacks["cucumber"] =
{
	title: "Cucumber",
	syntax: "cucumber",
	filename: "*.feature",
	insight: {
		syntax: "cucumber",
		entities: {
			"CucumberSource": {
				category: "Void"
			},
			"Feature": {
				category: "Module",
				identifier: "FeatureTitle"
			},
			"Tag": {
				category: "Variable",
				identifier: "Tag"
			},
			"Scenario": {
				category: "Class",
				identifier: "ScenarioTitle"
			}
		}
	}
};
