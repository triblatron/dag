root=
{
	items=
	{
		{
			cmd="COMMAND_CREATE_NODE",
			nodeClass="GroupTyped",
			nodeName="group1",
			assertions=
			{
				{
					path="graph.numNodes",
					value=1,
					typeIndex="TYPE_UINT",
					op="RELOP_EQ",
				},
			},
		},
		{
			cmd="COMMAND_CREATE_NODE",
			nodeClass="FooTyped",
			nodeName="foo1",
			assertions=
			{
				{
					path="graph.numNodes",
					value=2,
					typeIndex="TYPE_UINT",
					op="RELOP_EQ",
				},
			},
		},
		{
			cmd="COMMAND_CREATE_NODE",
			nodeClass="BarTyped",
			nodeName="bar1",
			assertions=
			{
				{
					path="graph.numNodes",
					value=3,
					typeIndex="TYPE_UINT",
					op="RELOP_EQ",
				},
			},
		},
		{
			cmd="COMMAND_CONNECT",
			fromPort=3,
			toPort=1,
			assertions=
			{
				{
					path="graph.ports[3].numOutgoingConnections",
					value=1,
					typeIndex="TYPE_UINT",
					op="RELOP_EQ",
				},
				{
					path="graph.ports[1].numIncomingConnections",
					value=1,
					typeIndex="TYPE_UINT",
					op="RELOP_EQ",
				},
			},
		},
		-- {
			-- cmd="COMMAND_SELECT",
			-- selection=
			-- {
				-- 0,
				-- 1,
				-- 2,
			-- },
			-- assertions=
			-- {
				-- {
					-- path="numSelectedNodes",
					-- value=3,
					-- op="RELOP_EQ",
				-- },
			-- },
		-- },
	}
}
