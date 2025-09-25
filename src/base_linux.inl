#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


// =================================
// ======= Custom Allocators =======
// =================================

HeapAllocator::HeapAllocator()
{
}

NoType* HeapAllocator::Alloc(Usize size)
{
    return malloc(size);
}

NoType* HeapAllocator::Realloc(NoType* ptr, Usize size)
{
    return realloc(ptr, size);
}

NoType HeapAllocator::Free(NoType* ptr)
{
    free(ptr);
}


// =======================
// ======= Threads =======
// =======================

Thread::Thread(ThreadFunc thrdFunc, NoType* data, Bool start)
{
    // TODO: We want the thread to start paused if 'start' is false
    pthread_create(&m_thread, nullptr, thrdFunc, data);
}

Thread::~Thread()
{
    // Not needed on linux (apparently)
}

NoType Thread::Run()
{
    // TODO:
}

NoType Thread::Stop()
{
    // TODO:
}

NoType Thread::Join()
{
    pthread_join(m_thread, nullptr);
}

NoType Thread::AssignCore(U32 core)
{
    // TODO:
}

NoType Thread::JoinMultiple(Thread* thrds, U32 thrdCount)
{
    for (U32 i = 0; i < thrdCount; i += 1)
        pthread_join(thrds[i].m_thread, nullptr);
}

NoType Thread::Exit(U32 code)
{
	pthread_exit(reinterpret_cast<void*>(code));
}

Mutex::Mutex(U32)
{
    pthread_mutex_init(&m_mutex, nullptr);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&m_mutex);
}

NoType Mutex::Lock()
{
    pthread_mutex_lock(&m_mutex);
}

Bool Mutex::TryLock()
{
    return pthread_mutex_trylock(&m_mutex) == 0;
}

NoType Mutex::Unlock()
{
    pthread_mutex_unlock(&m_mutex);
}


// =============================
// ======= Miscellaneous =======
// =============================

void _AssertRel(const C8* msg, const C8* file, const U32 line)
{
    const C8* yadCmd = "yad --title=\"Error\" --text=\"%s\" --button=OK:0 --fixed --window-icon=error --image=dialog-error --center";
    const C8* zenityCmd = "zenity --text=\"%s\" --error";
    C8* cmd = nullptr;

    C8 finalMsg[512] = {};
    C8 finalCmd[1024] = {};

    snprintf(finalMsg, SIZE_OF(finalMsg), "%s\n%s:%u\n", msg, file, line);

    if (system("yad --help > /dev/null") == 0)
        cmd = const_cast<C8*>(yadCmd);
    else if (system("zenity --help > /dev/null") == 0)
        cmd = const_cast<C8*>(zenityCmd);
    else
    {
        // If we can't display a dialog window just print it on stderr
        fprintf(stderr, "\033[31mError\033[0m: %s\n", finalMsg);
        exit(-1);
    }

    snprintf(finalCmd, SIZE_OF(finalCmd), cmd, finalMsg);
    system(finalCmd);

    exit(-1);
}

int main(int argc, char* argv[])
{
	return EntryPoint(argc, argv);
}
