root=
{
	items=
	{
		{
			cmd="COMMAND_CREATE_NODE",
			nodeClass="GroupTyped",
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
			cmd="COMMAND_LOAD_SCRIPTS",
			filename="etc/tests/NodeEditorLive/scripts.lua",
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
			cmd="COMMAND_ATTACH_SCRIPT",
			funcName="foo",
		},
		--~ {
			--~ cmd="COMMAND_CREATE_TEMPLATE",
			--~ nodeClass="Test",
			--~ status=
			--~ {
				--~ statusCode="STATUS_OK",
			--~ },
			--~ assertions=
			--~ {
			--~ }
		--~ },
		--~ {
			--~ cmd="COMMAND_CREATE_NODE",
			--~ nodeClass="Test",
			--~ nodeName="test1",
			--~ status=
			--~ {
				--~ statusCode="STATUS_OK",
				--~ resultType="RESULT_NODE_ID",
 				--~ nodeID=1,
			--~ },
			--~ assertions=
			--~ {
				--~ {
					--~ path="activeGraph.numNodes",
					--~ value=2,
					--~ typeIndex="TYPE_UINT",
					--~ op="RELOP_EQ",
				--~ },
			--~ },

		--~ },
		--~ {
			--~ cmd="COMMAND_SELECT",
			--~ selection=
			--~ {
				--~ 0,
				--~ 1,
			--~ },
			--~ selectionMode="SELECTION_SET",
			--~ assertions=
			--~ {
				--~ {
					--~ path="numSelectedNodes",
					--~ value=2,
					--~ typeIndex="TYPE_UINT",
					--~ op="RELOP_EQ",
				--~ },
			--~ },
		--~ },
		--~ {
			--~ cmd="COMMAND_COMPARE_NODES",
			--~ status=
			--~ {
				--~ statusCode="STATUS_OK",
			--~ },
			--~ assertions=
			--~ {
			--~ },
		--~ },
		{
			cmd="COMMAND_EVALUATE_GRAPH",
			existing=
			{
				{
					class="Port",
					properties=
					{
						parentClass="GroupTyped",
						value=3.0,
					},
				},
			},
		},
	},
}
