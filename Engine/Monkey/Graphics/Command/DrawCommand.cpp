#include "DrawCommand.h"

MeshDrawCommand* MeshDrawListContext::AddCommand(MeshDrawCommand* command)
{
	m_DrawCommandList.push_back(command);
	return command;
}