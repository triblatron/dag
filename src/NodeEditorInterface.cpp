#include "config/config.h"

#include "NodeEditorInterface.h"
#include "util/enums.h"

namespace dag
{
	const char* NodeEditorInterface::selectionModeToString(SelectionMode value)
	{
		switch (value)
		{
			ENUM_NAME(SELECTION_UNKNOWN)
			ENUM_NAME(SELECTION_SET)
			ENUM_NAME(SELECTION_ADD)
			ENUM_NAME(SELECTION_SUBTRACT)
			ENUM_NAME(SELECTION_TOGGLE)
		    ENUM_NAME(SELECTION_CLEAR)
		}

		return "<error>";
	}

	NodeEditorInterface::SelectionMode NodeEditorInterface::parseSelectionMode(const char* str)
	{
		TEST_ENUM(SELECTION_UNKNOWN, str);
		TEST_ENUM(SELECTION_SET, str);
		TEST_ENUM(SELECTION_ADD, str);
		TEST_ENUM(SELECTION_SUBTRACT, str);
		TEST_ENUM(SELECTION_TOGGLE, str);
        TEST_ENUM(SELECTION_CLEAR, str);

		return SELECTION_UNKNOWN;
	}
}