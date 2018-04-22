#include "RenderStatistics.h"

namespace ncine {

///////////////////////////////////////////////////////////
// STATIC DEFINITIONS
///////////////////////////////////////////////////////////

nctl::String RenderStatistics::debugString_(256);
RenderStatistics::Commands RenderStatistics::allCommands_;
RenderStatistics::Commands RenderStatistics::typedCommands_[RenderCommand::CommandTypes::COUNT];
RenderStatistics::Buffers RenderStatistics::typedBuffers_[RenderBuffersManager::BufferTypes::COUNT];
RenderStatistics::Textures RenderStatistics::textures_;
RenderStatistics::CustomVbos RenderStatistics::customVbos_;
unsigned int RenderStatistics::index = 0;
unsigned int RenderStatistics::culledNodes_[2] = {0, 0};
RenderStatistics::VaoPool RenderStatistics::vaoPool_;

///////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////

void RenderStatistics::appendCommandsStatistics(nctl::String &string)
{
	const Commands &spriteCommands = typedCommands_[RenderCommand::CommandTypes::SPRITE];
	const Commands &particleCommands = typedCommands_[RenderCommand::CommandTypes::PARTICLE];
	const Commands &textCommands = typedCommands_[RenderCommand::CommandTypes::TEXT];
	const Commands &plotterCommands = typedCommands_[RenderCommand::CommandTypes::PLOTTER];

	string.formatAppend(
		"Sprites: %uV, %uDC (%u Tr), %uI\n"\
		"Particles: %uV, %uDC (%u Tr), %uI\n"\
		"Text: %uV, %uDC (%u Tr), %uI\n"\
		"Plotter: %uV, %uDC (%u Tr), %uI\n"\
		"Total: %uV, %uDC (%u Tr), %uI\n",
		spriteCommands.vertices, spriteCommands.commands, spriteCommands.transparents, spriteCommands.instances,
		particleCommands.vertices, particleCommands.commands, particleCommands.transparents, particleCommands.instances,
		textCommands.vertices, textCommands.commands, textCommands.transparents, textCommands.instances,
		plotterCommands.vertices, plotterCommands.commands, plotterCommands.transparents, plotterCommands.instances,
		allCommands_.vertices, allCommands_.commands, allCommands_.transparents, allCommands_.instances);
}

void RenderStatistics::appendMoreStatistics(nctl::String &string)
{
	const Buffers &vboBuffers = typedBuffers_[RenderBuffersManager::BufferTypes::ARRAY];
	const Buffers &uboBuffers = typedBuffers_[RenderBuffersManager::BufferTypes::UNIFORM];

	string.formatAppend(
		"Culled nodes: %u\n"\
		"%u/%u VAOs (%u reuses, %u bindings)\n"\
		"%.2f Kb in %u Texture(s)\n"\
		"%.2f Kb in %u custom VBO(s)\n"\
		"%.2f/%u Kb in %u VBO(s)\n"\
		"%.2f/%u Kb in %u UBO(s)\n",
		culledNodes_[(index + 1) % 2],
		vaoPool_.size, vaoPool_.capacity, vaoPool_.reuses, vaoPool_.bindings,
		textures_.dataSize / 1024.0f, textures_.count,
		customVbos_.dataSize / 1024.0f, customVbos_.count,
		vboBuffers.usedSpace / 1024.0f, vboBuffers.size / 1024, vboBuffers.count,
		uboBuffers.usedSpace / 1024.0f, uboBuffers.size / 1024, uboBuffers.count);
}


///////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////

void RenderStatistics::reset()
{
	for (unsigned int i = 0; i < RenderCommand::CommandTypes::COUNT; i++)
		typedCommands_[i].reset();
	allCommands_.reset();

	for (unsigned int i = 0; i < RenderBuffersManager::BufferTypes::COUNT; i++)
		typedBuffers_[i].reset();

	// Ping pong index for last and current frame
	index = (index + 1) % 2;
	culledNodes_[index] = 0;

	vaoPool_.reset();
}

void RenderStatistics::gatherStatistics(const RenderCommand &command)
{
	const unsigned int typeIndex = command.type();
	typedCommands_[typeIndex].vertices += (command.numInstances() > 0) ?
		command.geometry().numVertices() * command.numInstances() : command.geometry().numVertices();
	typedCommands_[typeIndex].commands++;
	typedCommands_[typeIndex].transparents += (command.material().isTransparent()) ? 1 : 0;
	typedCommands_[typeIndex].instances += command.numInstances();

	allCommands_.vertices += command.geometry().numVertices();
	allCommands_.commands++;
	allCommands_.transparents += (command.material().isTransparent()) ? 1 : 0;
	allCommands_.instances += command.numInstances();
}

void RenderStatistics::gatherStatistics(const RenderBuffersManager::ManagedBuffer &buffer)
{
	const unsigned int typeIndex = buffer.type;
	typedBuffers_[typeIndex].count++;
	typedBuffers_[typeIndex].size += buffer.size;
	typedBuffers_[typeIndex].usedSpace += buffer.size - buffer.freeSpace;
}

}
