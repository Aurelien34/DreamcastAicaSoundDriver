#include "common.h"

#include "CommandQueue.h"

// The static command queue
static volatile AicaCommandQueue_t *g_pCommandQueue = (AicaCommandQueue_t *) AICAD_COMMAND_QUEUE;

static char g_pCommand[AICAD_COMMAND_PARAMETERS_LENGTH];

// Initialise the command queue
void InitCommandQueue()
{
    g_pCommandQueue->stQueuePtrs.nReadPtr = 0;
    g_pCommandQueue->stQueuePtrs.nWritePtr = 0;
}

// Read a command (returns NULL if no command found)
short ReadCommand()
{
    short nCommand = AICAD_COMMAND_ID_NOP;
    // Is there a command in the queue?
    if (g_pCommandQueue->stQueuePtrs.nReadPtr != g_pCommandQueue->stQueuePtrs.nWritePtr)
    {
        // There is a command
        // Get the command pointer
        char * pCommand = (char *) g_pCommandQueue->pCommandsList[g_pCommandQueue->stQueuePtrs.nReadPtr];
        // Read the command type and size
        uint uRead = *((uint*)pCommand);
        // Compute the command ID
        nCommand = AICAD_COMMAND_GET_COMMAND_ID(uRead);
        // Compute the command parameters size
        int nParamSize = AICAD_COMMAND_GET_PARAMETERS_SIZE(uRead);
        // Read the parameters
        if (nParamSize > 0)
        {
            memcpy(g_pCommand, pCommand + 4, nParamSize);
        }
        // Set the read pointer's position
        g_pCommandQueue->stQueuePtrs.nReadPtr = (g_pCommandQueue->stQueuePtrs.nReadPtr + 1) % AICAD_COMMAND_QUEUE_COMMAND_COUNT;
    }
    return nCommand;
}

// Return the command parameters, read by the last ReadCommand function
char * GetCommandParameters()
{
    return g_pCommand;
}
