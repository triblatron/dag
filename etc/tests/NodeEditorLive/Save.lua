root=
{
	cases=
	{
		{
			name="Saving a Base Node",
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
	},
	items=
	{
		{
			cmd="COMMAND_CREATE_NODE",
			nodeClass="BarTyped",
			nodeName="",
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
			cmd="COMMAND_SET_POSITION",
			node=0,
			x=100.0,
			y=200.0,
		},
		{
			cmd="COMMAND_SAVE",
		},
	},
}
