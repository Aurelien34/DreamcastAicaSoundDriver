#include "common.h"

// The static message queue
static volatile AicaMessageQueue_t *g_pMessageQueue = (AicaMessageQueue_t *) AICAD_MESSAGE_QUEUE;

// Initialise the message queue
void InitMessageQueue()
{
    g_pMessageQueue->stQueuePtrs.nReadPtr = 0;
    g_pMessageQueue->stQueuePtrs.nWritePtr = 0;
}

// Make a blocking message send
void SendMessage(const char *szMessage)
{
    // Loop until there is still free space for the new message
    while (
        (g_pMessageQueue->stQueuePtrs.nReadPtr == 0 && g_pMessageQueue->stQueuePtrs.nWritePtr == AICAD_MESSAGE_QUEUE_MESSAGE_COUNT - 1 )
        ||
        (g_pMessageQueue->stQueuePtrs.nWritePtr == g_pMessageQueue->stQueuePtrs.nReadPtr - 1))
    {
        ;
    }

    // Copy the message in the queue
    memcpy(
        &(g_pMessageQueue->pMessagesList[
            g_pMessageQueue->stQueuePtrs.nWritePtr]),
        szMessage,
        AICAD_MESSAGE_QUEUE_MESSAGE_LENGTH);
    // Increment the write ptr
    g_pMessageQueue->stQueuePtrs.nWritePtr = (g_pMessageQueue->stQueuePtrs.nWritePtr + 1) % AICAD_MESSAGE_QUEUE_MESSAGE_COUNT;
}
