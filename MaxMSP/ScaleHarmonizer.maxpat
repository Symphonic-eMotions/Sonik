{
	"patcher" : 	{
		"fileversion" : 1,
		"appversion" : 		{
			"major" : 8,
			"minor" : 6,
			"revision" : 5,
			"architecture" : "x64",
			"modernui" : 1
		}
,
		"classnamespace" : "box",
		"rect" : [ 464.0, 169.0, 679.0, 753.0 ],
		"bglocked" : 0,
		"openinpresentation" : 0,
		"default_fontsize" : 12.0,
		"default_fontface" : 0,
		"default_fontname" : "Arial",
		"gridonopen" : 1,
		"gridsize" : [ 15.0, 15.0 ],
		"gridsnaponopen" : 1,
		"objectsnaponopen" : 1,
		"statusbarvisible" : 2,
		"toolbarvisible" : 1,
		"lefttoolbarpinned" : 0,
		"toptoolbarpinned" : 0,
		"righttoolbarpinned" : 0,
		"bottomtoolbarpinned" : 0,
		"toolbars_unpinned_last_save" : 0,
		"tallnewobj" : 0,
		"boxanimatetime" : 200,
		"enablehscroll" : 1,
		"enablevscroll" : 1,
		"devicewidth" : 0.0,
		"description" : "",
		"digest" : "",
		"tags" : "",
		"style" : "",
		"subpatcher_template" : "",
		"assistshowspatchername" : 0,
		"boxes" : [ 			{
				"box" : 				{
					"id" : "obj-2",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 2,
					"outlettype" : [ "float", "bang" ],
					"patching_rect" : [ 79.0, 181.0, 29.5, 22.0 ],
					"text" : "t f b"
				}

			}
, 			{
				"box" : 				{
					"autosave" : 1,
					"id" : "obj-rnbo",
					"inletInfo" : 					{
						"IOInfo" : [ 							{
								"type" : "midi",
								"index" : -1,
								"tag" : "",
								"comment" : ""
							}
 ]
					}
,
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outletInfo" : 					{
						"IOInfo" : [  ]
					}
,
					"outlettype" : [ "list" ],
					"patcher" : 					{
						"fileversion" : 1,
						"appversion" : 						{
							"major" : 8,
							"minor" : 6,
							"revision" : 5,
							"architecture" : "x64",
							"modernui" : 1
						}
,
						"classnamespace" : "rnbo",
						"rect" : [ 84.0, 131.0, 640.0, 480.0 ],
						"bglocked" : 0,
						"openinpresentation" : 0,
						"default_fontsize" : 12.0,
						"default_fontface" : 0,
						"default_fontname" : "Lato",
						"gridonopen" : 1,
						"gridsize" : [ 15.0, 15.0 ],
						"gridsnaponopen" : 1,
						"objectsnaponopen" : 1,
						"statusbarvisible" : 2,
						"toolbarvisible" : 1,
						"lefttoolbarpinned" : 0,
						"toptoolbarpinned" : 0,
						"righttoolbarpinned" : 0,
						"bottomtoolbarpinned" : 0,
						"toolbars_unpinned_last_save" : 0,
						"tallnewobj" : 0,
						"boxanimatetime" : 200,
						"enablehscroll" : 1,
						"enablevscroll" : 1,
						"devicewidth" : 0.0,
						"description" : "",
						"digest" : "",
						"tags" : "",
						"style" : "",
						"subpatcher_template" : "",
						"assistshowspatchername" : 0,
						"title" : "untitled",
						"boxes" : [ 							{
								"box" : 								{
									"id" : "obj-param-harmonicity",
									"maxclass" : "newobj",
									"numinlets" : 2,
									"numoutlets" : 2,
									"outlettype" : [ "", "" ],
									"patching_rect" : [ 91.5, 136.0, 262.0, 23.0 ],
									"rnbo_classname" : "param",
									"rnbo_extra_attributes" : 									{
										"fromnormalized" : "",
										"exponent" : 1.0,
										"displayname" : "",
										"enum" : "",
										"meta" : "",
										"preset" : 1,
										"sendinit" : 1,
										"unit" : "",
										"steps" : 0.0,
										"order" : "0",
										"displayorder" : "-",
										"ctlin" : -1.0,
										"tonormalized" : ""
									}
,
									"rnbo_serial" : 1,
									"rnbo_uniqueid" : "harmonicity",
									"rnboinfo" : 									{
										"needsInstanceInfo" : 1,
										"argnames" : 										{
											"value" : 											{
												"attrOrProp" : 1,
												"digest" : "Parameter value",
												"defaultarg" : 2,
												"isalias" : 0,
												"aliases" : [  ],
												"settable" : 1,
												"attachable" : 0,
												"isparam" : 1,
												"deprecated" : 0,
												"touched" : 0,
												"inlet" : 1,
												"type" : "number",
												"defaultValue" : "1"
											}
,
											"normalizedvalue" : 											{
												"attrOrProp" : 1,
												"digest" : "Set value normalized. ",
												"isalias" : 0,
												"aliases" : [  ],
												"attachable" : 0,
												"isparam" : 0,
												"deprecated" : 0,
												"touched" : 0,
												"inlet" : 1,
												"type" : "number"
											}
,
											"reset" : 											{
												"attrOrProp" : 1,
												"digest" : "Reset param to initial value",
												"isalias" : 0,
												"aliases" : [  ],
												"attachable" : 1,
												"isparam" : 0,
												"deprecated" : 0,
												"touched" : 0,
												"type" : "bang"
											}
,
											"normalized" : 											{
												"attrOrProp" : 1,
												"digest" : "Normalized parameter value.",
												"isalias" : 0,
												"aliases" : [  ],
												"attachable" : 0,
												"isparam" : 0,
												"deprecated" : 0,
												"touched" : 0,
												"outlet" : 1,
												"type" : "number"
											}
,
											"name" : 											{
												"attrOrProp" : 2,
												"digest" : "Name of the parameter",
												"defaultarg" : 1,
												"isalias" : 0,
												"aliases" : [  ],
												"settable" : 1,
												"attachable" : 0,
												"isparam" : 0,
												"deprecated" : 0,
												"touched" : 0,
												"type" : "symbol",
												"label" : "Parameter Name",
												"mandatory" : 1
											}
,
											"enum" : 											{
												"attrOrProp" : 2,
												"digest" : "Use an enumerated output",
												"isalias" : 0,
												"aliases" : [  ],
												"settable" : 1,
												"attachable" : 0,
												"isparam" : 0,
												"deprecated" : 0,
												"touched" : 0,
												"type" : "list",
												"label" : "Enum Values",
												"displayorder" : 6
											}
,
											"minimum" : 											{
												"attrOrProp" : 2,
												"digest" : "Minimum value",
												"isalias" : 0,
												"aliases" : [ "min" ],
												"settable" : 1,
												"attachable" : 0,
												"isparam" : 0,
												"deprecated" : 0,
												"touched" : 0,
												"type" : "number",
												"defaultValue" : "0",
												"label" : "Minimum",
												"displayorder" : 1
											}
,
											"min" : 											{
												"attrOrProp" : 2,
												"digest" : "Minimum value",
												"isalias" : 1,
												"aliasOf" : "minimum",
												"aliases" : [  ],
												"settable" : 1,
												"attachable" : 0,
												"isparam" : 0,
												"deprecated" : 0,
												"touched" : 0,
												"type" : "number",
												"defaultValue" : "0",
												"label" : "Minimum",
												"displayorder" : 1
											}
,
											"maximum" : 											{
												"attrOrProp" : 2,
												"digest" : "Maximum value",
												"isalias" : 0,
												"aliases" : [ "max" ],
												"settable" : 1,
												"attachable" : 0,
												"isparam" : 0,
												"deprecated" : 0,
												"touched" : 0,
												"type" : "number",
												"defaultValue" : "1",
												"label" : "Maximum",
												"displayorder" : 2
											}
,
											"max" : 											{
												"attrOrProp" : 2,
												"digest" : "Maximum value",
												"isalias" : 1,
												"aliasOf" : "maximum",
												"aliases" : [  ],
												"settable" : 1,
												"attachable" : 0,
												"isparam" : 0,
												"deprecated" : 0,
												"touched" : 0,
												"type" : "number",
												"defaultValue" : "1",
												"label" : "Maximum",
												"displayorder" : 2
											}
,
											"exponent" : 											{
												"attrOrProp" : 2,
												"digest" : "Scale values exponentially",
												"isalias" : 0,
												"aliases" : [  ],
												"settable" : 1,
												"attachable" : 0,
												"isparam" : 0,
												"deprecated" : 0,
												"touched" : 0,
												"type" : "number",
												"defaultValue" : "1",
												"label" : "Exponent",
												"displayorder" : 7
											}
,
											"steps" : 											{
												"attrOrProp" : 2,
												"digest" : "Divide the output into a number of discrete steps",
												"isalias" : 0,
												"aliases" : [  ],
												"settable" : 1,
												"attachable" : 0,
												"isparam" : 0,
												"deprecated" : 0,
												"touched" : 0,
												"type" : "number",
												"defaultValue" : "0",
												"label" : "Steps",
												"displayorder" : 8
											}
,
											"displayName" : 											{
												"attrOrProp" : 2,
												"digest" : "DEPRECATED: Use the lower case 'displayname' instead",
												"isalias" : 0,
												"aliases" : [  ],
												"settable" : 1,
												"attachable" : 0,
												"isparam" : 0,
												"deprecated" : 1,
												"touched" : 0,
												"type" : "symbol",
												"label" : "Display Name"
											}
,
											"displayname" : 											{
												"attrOrProp" : 2,
												"digest" : "A more readable name for the parameter in an external RNBO target",
												"isalias" : 0,
												"aliases" : [  ],
												"settable" : 1,
												"attachable" : 0,
												"isparam" : 0,
												"deprecated" : 0,
												"touched" : 0,
												"type" : "symbol",
												"defaultValue" : "",
												"label" : "Display Name",
												"displayorder" : 14
											}
,
											"unit" : 											{
												"attrOrProp" : 2,
												"digest" : "A symbol to describe the unit of the parameter in an external RNBO target",
												"isalias" : 0,
												"aliases" : [  ],
												"settable" : 1,
												"attachable" : 0,
												"isparam" : 0,
												"deprecated" : 0,
												"touched" : 0,
												"type" : "symbol",
												"defaultValue" : "",
												"label" : "Unit",
												"displayorder" : 15
											}
,
											"tonormalized" : 											{
												"attrOrProp" : 2,
												"digest" : "Converts a real parameter value to its normalized form",
												"isalias" : 0,
												"aliases" : [  ],
												"settable" : 1,
												"attachable" : 0,
												"isparam" : 0,
												"deprecated" : 0,
												"touched" : 0,
												"type" : "symbol",
												"label" : "To Normalized Expression",
												"displayorder" : 10
											}
,
											"fromnormalized" : 											{
												"attrOrProp" : 2,
												"digest" : "Converts a normalized parameter into its actual parameter value",
												"isalias" : 0,
												"aliases" : [  ],
												"settable" : 1,
												"attachable" : 0,
												"isparam" : 0,
												"deprecated" : 0,
												"touched" : 0,
												"type" : "symbol",
												"label" : "From Normalized Expression",
												"displayorder" : 9
											}
,
											"order" : 											{
												"attrOrProp" : 2,
												"digest" : "Order in which initial parameter values will be sent out on patcher load. The order can be numeric or symbolic ('first' and 'last')",
												"isalias" : 0,
												"aliases" : [  ],
												"settable" : 1,
												"attachable" : 0,
												"isparam" : 0,
												"deprecated" : 0,
												"touched" : 0,
												"type" : "symbol",
												"defaultValue" : "0",
												"label" : "Restore Order",
												"displayorder" : 12
											}
,
											"displayorder" : 											{
												"attrOrProp" : 2,
												"digest" : "Order in which parameters will show up in a list of all parameters. The order can be numeric or symbolic ('first' and 'last')",
												"isalias" : 0,
												"aliases" : [  ],
												"settable" : 1,
												"attachable" : 0,
												"isparam" : 0,
												"deprecated" : 0,
												"touched" : 0,
												"type" : "symbol",
												"defaultValue" : "-",
												"label" : "Display Order",
												"displayorder" : 13
											}
,
											"sendinit" : 											{
												"attrOrProp" : 2,
												"digest" : "Send initial value",
												"isalias" : 0,
												"aliases" : [  ],
												"settable" : 1,
												"attachable" : 0,
												"isparam" : 0,
												"deprecated" : 0,
												"touched" : 0,
												"type" : "bool",
												"defaultValue" : "true",
												"label" : "Send Init",
												"displayorder" : 4
											}
,
											"ctlin" : 											{
												"attrOrProp" : 2,
												"digest" : "MIDI controller number to control this parameter.",
												"isalias" : 0,
												"aliases" : [  ],
												"settable" : 1,
												"attachable" : 0,
												"isparam" : 0,
												"deprecated" : 0,
												"touched" : 0,
												"type" : "number",
												"defaultValue" : "-1",
												"label" : "MIDI Controller Number.",
												"displayorder" : 16
											}
,
											"meta" : 											{
												"attrOrProp" : 2,
												"digest" : "A JSON formatted string containing metadata for use by the exported code",
												"isalias" : 0,
												"aliases" : [  ],
												"settable" : 1,
												"attachable" : 0,
												"isparam" : 0,
												"deprecated" : 0,
												"touched" : 0,
												"type" : "symbol",
												"defaultValue" : "",
												"label" : "Metadata",
												"displayorder" : 17
											}
,
											"nopreset" : 											{
												"attrOrProp" : 2,
												"digest" : "Do not add this value to the preset [DEPRECATED - USE @preset 0 instead].",
												"isalias" : 0,
												"aliases" : [  ],
												"settable" : 1,
												"attachable" : 0,
												"isparam" : 0,
												"deprecated" : 1,
												"touched" : 0,
												"type" : "bool",
												"defaultValue" : "false"
											}
,
											"preset" : 											{
												"attrOrProp" : 2,
												"digest" : "Add this value to the preset.",
												"isalias" : 0,
												"aliases" : [  ],
												"settable" : 1,
												"attachable" : 0,
												"isparam" : 0,
												"deprecated" : 0,
												"touched" : 0,
												"type" : "bool",
												"defaultValue" : "true",
												"label" : "Include In Preset",
												"displayorder" : 11
											}

										}
,
										"inputs" : [ 											{
												"name" : "value",
												"type" : "number",
												"digest" : "Parameter value",
												"defaultarg" : 2,
												"hot" : 1,
												"docked" : 0
											}
, 											{
												"name" : "normalizedvalue",
												"type" : "number",
												"digest" : "Set value normalized. ",
												"docked" : 0
											}
 ],
										"outputs" : [ 											{
												"name" : "value",
												"type" : "number",
												"digest" : "Parameter value",
												"defaultarg" : 2,
												"hot" : 1,
												"docked" : 0
											}
, 											{
												"name" : "normalized",
												"type" : "number",
												"digest" : "Normalized parameter value.",
												"docked" : 0
											}
 ],
										"helpname" : "param",
										"aliasOf" : "param",
										"classname" : "param",
										"operator" : 0,
										"versionId" : -1661410411,
										"changesPatcherIO" : 0
									}
,
									"text" : "param harmonicity @value 1 @min 1 @max 10",
									"varname" : "harmonicity"
								}

							}
, 							{
								"box" : 								{
									"id" : "obj-notein",
									"maxclass" : "newobj",
									"numinlets" : 2,
									"numoutlets" : 4,
									"outlettype" : [ "", "", "", "" ],
									"patching_rect" : [ 40.0, 19.0, 50.5, 23.0 ],
									"rnbo_classname" : "notein",
									"rnbo_serial" : 1,
									"rnbo_uniqueid" : "notein_obj-notein",
									"text" : "notein"
								}

							}
 ],
						"lines" : [  ],
						"default_bgcolor" : [ 0.031372549019608, 0.125490196078431, 0.211764705882353, 1.0 ],
						"color" : [ 0.929412, 0.929412, 0.352941, 1.0 ],
						"elementcolor" : [ 0.357540726661682, 0.515565991401672, 0.861786782741547, 1.0 ],
						"accentcolor" : [ 0.343034118413925, 0.506230533123016, 0.86220508813858, 1.0 ],
						"stripecolor" : [ 0.258338063955307, 0.352425158023834, 0.511919498443604, 1.0 ],
						"bgfillcolor_type" : "color",
						"bgfillcolor_color" : [ 0.031372549019608, 0.125490196078431, 0.211764705882353, 1.0 ],
						"bgfillcolor_color1" : [ 0.031372549019608, 0.125490196078431, 0.211764705882353, 1.0 ],
						"bgfillcolor_color2" : [ 0.263682, 0.004541, 0.038797, 1.0 ],
						"bgfillcolor_angle" : 270.0,
						"bgfillcolor_proportion" : 0.39,
						"bgfillcolor_autogradient" : 0.0
					}
,
					"patching_rect" : [ 79.0, 439.0, 40.0, 22.0 ],
					"rnboattrcache" : 					{
						"harmonicity" : 						{
							"label" : "harmonicity",
							"isEnum" : 0,
							"parsestring" : ""
						}

					}
,
					"rnboversion" : "1.4.1",
					"saved_attribute_attributes" : 					{
						"valueof" : 						{
							"parameter_invisible" : 1,
							"parameter_longname" : "rnbo~",
							"parameter_modmode" : 0,
							"parameter_shortname" : "rnbo~",
							"parameter_type" : 3
						}

					}
,
					"saved_object_attributes" : 					{
						"optimization" : "O1",
						"parameter_enable" : 1,
						"uuid" : "adba0765-8aeb-11f0-8765-acde48001122"
					}
,
					"snapshot" : 					{
						"filetype" : "C74Snapshot",
						"version" : 2,
						"minorversion" : 0,
						"name" : "snapshotlist",
						"origin" : "rnbo~",
						"type" : "list",
						"subtype" : "Undefined",
						"embed" : 1,
						"snapshot" : 						{
							"harmonicity" : 							{
								"value" : 1.0
							}
,
							"__presetid" : "adba0765-8aeb-11f0-8765-acde48001122"
						}
,
						"snapshotlist" : 						{
							"current_snapshot" : 0,
							"entries" : [ 								{
									"filetype" : "C74Snapshot",
									"version" : 2,
									"minorversion" : 0,
									"name" : "untitled",
									"origin" : "adba0765-8aeb-11f0-8765-acde48001122",
									"type" : "rnbo",
									"subtype" : "",
									"embed" : 0,
									"snapshot" : 									{
										"harmonicity" : 										{
											"value" : 1.0
										}
,
										"__presetid" : "adba0765-8aeb-11f0-8765-acde48001122"
									}
,
									"fileref" : 									{
										"name" : "untitled",
										"filename" : "untitled_20250906.maxsnap",
										"filepath" : "~/Documents/Max 8/Snapshots",
										"filepos" : -1,
										"snapshotfileid" : "1a3605f3d18ca9de090b26a78b208a5c"
									}

								}
 ]
						}

					}
,
					"text" : "rnbo~",
					"varname" : "rnbo~"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-scale-menu",
					"items" : [ "Equal (12-TET)", ",", "Just (5-limit)", ",", "Pythagorean", ",", "Meantone (1/4 comma)", ",", "Harmonic Series", ",", "Hungarian Gypsy" ],
					"maxclass" : "umenu",
					"numinlets" : 1,
					"numoutlets" : 3,
					"outlettype" : [ "int", "", "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 79.0, 105.0, 190.0, 22.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-harmo-menu",
					"items" : [ 1.059463, ",", 1.122462, ",", 1.189207, ",", 1.259921, ",", 1.33484, ",", 1.414214, ",", 1.498307, ",", 1.587401, ",", 1.681793, ",", 1.781797, ",", 1.887749 ],
					"maxclass" : "umenu",
					"numinlets" : 1,
					"numoutlets" : 3,
					"outlettype" : [ "int", "", "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 79.0, 323.0, 150.0, 22.0 ]
				}

			}
, 			{
				"box" : 				{
					"coll_data" : 					{
						"count" : 12,
						"data" : [ 							{
								"key" : 1,
								"value" : [ 1.059463, 1.122462, 1.189207, 1.259921, 1.33484, 1.414214, 1.498307, 1.587401, 1.681793, 1.781797, 1.887749, 2.0, 2.118926, 2.244924, 2.378414, 2.519842, 2.66968, 2.828427, 2.996615, 3.174802, 3.363586, 3.563595, 3.775498, 4.0, 4.237853, 4.489849, 4.756828, 5.039684, 5.339373, 5.656854, 5.993165, 6.349604, 6.727171, 7.127189, 7.550505, 8.0, 8.470851, 8.9757, 9.414213 ]
							}
, 							{
								"key" : 2,
								"value" : [ 1.0, 1.066667, 1.125, 1.2, 1.25, 1.333333, 1.40625, 1.5, 1.6, 1.666667, 1.8, 1.875, 2.0, 2.133334, 2.25, 2.4, 2.5, 2.666666, 2.8125, 3.0, 3.2, 3.333334, 3.6, 3.75, 4.0, 4.266668, 4.5, 4.8, 5.0, 5.333332, 5.625, 6.0, 6.4, 6.666668, 7.2, 7.5, 8.0, 8.533336, 9.0, 9.6, 10.0 ]
							}
, 							{
								"key" : 3,
								"value" : [ 1.0, 1.053497, 1.125, 1.185185, 1.265625, 1.333333, 1.423828, 1.5, 1.580246, 1.6875, 1.777778, 1.898438, 2.0, 2.106994, 2.25, 2.37037, 2.53125, 2.666666, 2.847656, 3.0, 3.160492, 3.375, 3.555556, 3.796876, 4.0, 4.213988, 4.5, 4.74074, 5.0625, 5.333332, 5.695312, 6.0, 6.320984, 6.75, 7.111112, 7.593752, 8.0, 8.427975999999999, 9.0, 9.481479999999999, 10.125 ]
							}
, 							{
								"key" : 4,
								"value" : [ 1.0, 1.07177, 1.11803, 1.19628, 1.25, 1.33748, 1.39754, 1.49535, 1.58073, 1.67185, 1.78886, 1.86919, 2.0, 2.14354, 2.23606, 2.39256, 2.5, 2.67496, 2.79508, 2.9907, 3.16146, 3.37772, 3.73838, 3.86482, 4.0, 4.28708, 4.47212, 4.78512, 5.0, 5.34992, 5.59016, 5.9814, 6.32292, 6.75544, 7.47676, 7.72964, 8.0, 8.574159999999999, 8.944240000000001, 9.57024, 10.0 ]
							}
, 							{
								"key" : 5,
								"value" : [ 1.0, 1.08333, 1.25, 1.33333, 1.41667, 1.5, 1.58333, 1.66667, 1.75, 1.83333, 1.91667, 2.0, 2.08333, 2.16667, 2.25, 2.33333, 2.41667, 2.5, 2.58333, 2.66667, 2.75, 2.83333, 2.91667, 3.0, 3.08333, 3.16667, 3.25, 3.33333, 3.41667, 3.5, 3.58333, 3.66667, 3.75, 3.83333, 3.91667, 4.0, 4.08333, 4.16667, 4.25, 4.33333, 4.41667, 4.5, 4.58333, 4.66667, 4.75, 4.83333, 4.91667, 5.0, 5.08333, 5.16667, 5.25, 5.33333, 5.41667, 5.5, 5.58333, 5.66667, 5.75, 5.83333, 5.91667, 6.0, 6.08333, 6.16667, 6.25, 6.33333, 6.41667, 6.5, 6.58333, 6.66667, 6.75, 6.83333, 6.91667, 7.0, 7.08333, 7.16667, 7.25, 7.33333, 7.41667, 7.5, 7.58333, 7.66667, 7.75, 7.83333, 7.91667, 8.0, 8.08333, 8.16667, 8.25, 8.33333, 8.41667, 8.5, 8.58333, 8.66667, 8.75, 8.83333, 8.91667, 9.0, 9.08333, 9.16667, 9.25, 9.33333, 9.41667, 9.5, 9.58333, 9.66667, 9.75, 9.83333, 9.91667, 10.0 ]
							}
, 							{
								"key" : 6,
								"value" : [ 1.0, 1.05946, 1.18921, 1.33484, 1.49831, 1.5874, 1.7818, 1.88775, 2.0, 2.24492, 2.37841, 2.51984, 2.66968, 2.83848, 3.07473, 3.2457, 3.5636, 3.7755, 3.99998, 4.48984, 4.75682, 5.03968, 5.33936, 5.67696, 6.14946, 6.4914, 7.1272, 7.5509, 7.99996, 8.97968, 9.513640000000001, 10.0 ]
							}
, 							{
								"key" : 7,
								"value" : [ 1, 1.259921, 1.498307, 1.887749, 2.519842, 5.039684, 10.079368000000001 ]
							}
, 							{
								"key" : 8,
								"value" : [ 1, 1.25, 1.5, 2, 3, 4.5, 9 ]
							}
, 							{
								"key" : 9,
								"value" : [ 1, 1.265625, 1.423828, 1.777778, 2.847656, 4, 8.427975999999999 ]
							}
, 							{
								"key" : 10,
								"value" : [ 1, 1.11803, 1.25, 1.39754, 2, 3.16146, 8.944240000000001 ]
							}
, 							{
								"key" : 11,
								"value" : [ 1, 1.25, 1.5, 2, 3, 5, 10 ]
							}
, 							{
								"key" : 12,
								"value" : [ 1, 1.33484, 1.591, 2.004, 3.254, 4.48, 8.970000000000001 ]
							}
 ]
					}
,
					"id" : "obj-coll",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 4,
					"outlettype" : [ "", "", "", "" ],
					"patching_rect" : [ 79.0, 218.0, 80.0, 22.0 ],
					"saved_object_attributes" : 					{
						"embed" : 1,
						"precision" : 6
					}
,
					"text" : "coll scales"
				}

			}
, 			{
				"box" : 				{
					"fontsize" : 10.0,
					"id" : "obj-coll-help",
					"linecount" : 8,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 55.0, 499.0, 515.0, 96.0 ],
					"text" : "1: Equal (12-TET)\n1. 1. 1.059463 1.122462 1.189207 1.259921 1.33484 1.414214 1.498307 1.587401 1.681793 1.781797 1.887749;\n\n2: Just (5-limit)\n2. 1. 1.066667 1.125 1.2 1.25 1.333333 1.40625 1.5 1.6 1.666667 1.8 1.875;\n\n3: Pythagorean\n3. 1. 1.053497 1.125 1.185185 1.265625 1.333333 1.423828 1.5 1.580246 1.6875 1.777778 1.898438;"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-plus1",
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"patching_rect" : [ 79.0, 144.0, 30.0, 22.0 ],
					"text" : "+ 1"
				}

			}
, 			{
				"box" : 				{
					"id" : "msg-clear",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 34.0, 218.0, 40.0, 22.0 ],
					"text" : "clear"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-iter",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 79.0, 253.0, 40.0, 22.0 ],
					"text" : "iter"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-prepend-append",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 79.0, 285.0, 100.0, 22.0 ],
					"text" : "prepend append"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-tonumber",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 79.0, 362.0, 52.0, 22.0 ],
					"text" : "expr $f1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-prepend-harm",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 79.0, 402.0, 130.0, 22.0 ],
					"text" : "prepend harmonicity"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-load",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "bang" ],
					"patching_rect" : [ 77.0, 35.0, 60.0, 22.0 ],
					"text" : "loadbang"
				}

			}
, 			{
				"box" : 				{
					"id" : "msg-select-first",
					"maxclass" : "message",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 79.0, 71.0, 20.0, 22.0 ],
					"text" : "0"
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"destination" : [ "obj-harmo-menu", 0 ],
					"source" : [ "msg-clear", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-scale-menu", 0 ],
					"source" : [ "msg-select-first", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "msg-clear", 0 ],
					"source" : [ "obj-2", 1 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-coll", 0 ],
					"source" : [ "obj-2", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-iter", 0 ],
					"source" : [ "obj-coll", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-tonumber", 0 ],
					"source" : [ "obj-harmo-menu", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-prepend-append", 0 ],
					"source" : [ "obj-iter", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "msg-select-first", 0 ],
					"source" : [ "obj-load", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-2", 0 ],
					"source" : [ "obj-plus1", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-harmo-menu", 0 ],
					"source" : [ "obj-prepend-append", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-rnbo", 0 ],
					"source" : [ "obj-prepend-harm", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-plus1", 0 ],
					"source" : [ "obj-scale-menu", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-prepend-harm", 0 ],
					"source" : [ "obj-tonumber", 0 ]
				}

			}
 ],
		"parameters" : 		{
			"obj-rnbo" : [ "rnbo~", "rnbo~", 0 ],
			"parameterbanks" : 			{
				"0" : 				{
					"index" : 0,
					"name" : "",
					"parameters" : [ "-", "-", "-", "-", "-", "-", "-", "-" ]
				}

			}
,
			"inherited_shortname" : 1
		}
,
		"dependency_cache" : [ 			{
				"name" : "untitled_20250906.maxsnap",
				"bootpath" : "~/Documents/Max 8/Snapshots",
				"patcherrelativepath" : "../../../../../../Users/fjw/Documents/Max 8/Snapshots",
				"type" : "mx@s",
				"implicit" : 1
			}
 ],
		"autosave" : 0
	}

}
