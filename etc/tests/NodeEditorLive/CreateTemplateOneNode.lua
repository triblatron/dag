root=
{
	cases=
	{
		{
			name="Copying a Base Node",
			subs=
			{
				{
					commandIndex=0,
					name="nodeClass",
					value="Base",
				},
			},
			assertions=
			{
				--~ {
					--~ commandIndex=2,
					--~ assertionIndex=1,
					--~ value=2,
				--~ },
			},
		},
		{
			name="Copying a Derived Node",
			subs=
			{
				{
					commandIndex=0,
					name="nodeClass",
					value="Derived",
				},
			},
			assertions=
			{
				--~ {
					--~ commandIndex=2,
					--~ assertionIndex=1,
					--~ value=4,
				--~ },
			},			
		},
		{
			name="Copying a GroupTyped Node",
			subs=
			{
				{
					commandIndex=0,
					name="nodeClass",
					value="GroupTyped",
				},
			},
			assertions=
			{
				--~ {
					--~ commandIndex=2,
					--~ assertionIndex=1,
					--~ value=4,
				--~ },
			},
		},
		{
			name="Copying a MathsNode",
			subs=
			{
				{
					commandIndex=0,
					name="nodeClass",
					value="MathsNode",
				},
			},
			assertions=
			{
				--~ {
					--~ commandIndex=2,
					--~ assertionIndex=1,
					--~ value=6,
				--~ },
			},
		},
		{
			name="Copying a Final Node",
			subs=
			{
				{
					commandIndex=0,
					name="nodeClass",
					value="Final",
				},
			},
			assertions=
			{
				--~ {
					--~ commandIndex=2,
					--~ assertionIndex=1,
					--~ value=6,
				--~ },
			},
		},
		{
			name="Copying a BarTyped Node",
			subs=
			{
				{
					commandIndex=0,
					name="nodeClass",
					value="BarTyped",
				},
			},
			assertions=
			{
				--~ {
					--~ commandIndex=2,
					--~ assertionIndex=1,
					--~ value=2,
				--~ },
			},
		},
		{
			name="Copying a FooTyped Node",
			subs=
			{
				--~ {
					--~ commandIndex=0,
					--~ name="nodeClass",
					--~ value="FooTyped",
				--~ },
			},
			assertions=
			{
				--~ {
					--~ commandIndex=2,
					--~ assertionIndex=1,
					--~ value=2,
				--~ },
			},
		},
	},
	items=
	{
		{
			cmd="COMMAND_CREATE_NODE",
			nodeClass="",
			nodeName="group1",
			status=
			{
				statusCode="STATUS_OK",
				resultType="RESULT_NODE_ID",
 				nodeID=0,
			},
			assertions=
			{
				{
					path="activeGraph.numNodes",
					value=1,
					typeIndex="TYPE_UINT",
					op="RELOP_EQ",
				},
			},
		},
		{
			cmd="COMMAND_SELECT",
			selection=
			{
				0,
			},
			selectionMode="SELECTION_SET",
			assertions=
			{
				{
					path="numSelectedNodes",
					value=1,
					typeIndex="TYPE_UINT",
					op="RELOP_EQ",
				},
			},
		},
		{
			cmd="COMMAND_CREATE_TEMPLATE",
			nodeClass="Test",
			status=
			{
				statusCode="STATUS_OK",
			},
			assertions=
			{
				{
					path="nodeLib.classes.Test.id",
					value=9,
					op="RELOP_EQ",
				},
			}
		},
		{
			cmd="COMMAND_CREATE_NODE",
			nodeClass="Test",
			nodeName="test1",
			status=
			{
				statusCode="STATUS_OK",
				resultType="RESULT_NODE_ID",
 				nodeID=1,
			},
			assertions=
			{
				{
					path="activeGraph.numNodes",
					value=2,
					typeIndex="TYPE_UINT",
					op="RELOP_EQ",
				},
			},

		},
		{
			cmd="COMMAND_SELECT",
			selection=
			{
				0,
				1,
			},
			selectionMode="SELECTION_SET",
			assertions=
			{
				{
					path="numSelectedNodes",
					value=2,
					typeIndex="TYPE_UINT",
					op="RELOP_EQ",
				},
			},
		},
		{
			cmd="COMMAND_COMPARE_NODES",
			status=
			{
				statusCode="STATUS_OK",
			},
			assertions=
			{
			},
		},
	},
}
