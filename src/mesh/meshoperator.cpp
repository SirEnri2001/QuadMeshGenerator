#include "meshoperator.h"
#include "../thread_support/thread_support.h"

MeshOperator::MeshOperator(Mesh* mesh, MeshDisplay* display) : mesh(mesh), display(display)
{
    
}

void MeshOperator::create() {

}

MeshInteriorOperator::MeshInteriorOperator(Mesh* mesh, MeshDisplay* display) :
    MeshOperator(mesh, display) {

}

MeshUserOperator::MeshUserOperator(Mesh* mesh, MeshDisplay* display) :
    MeshOperator(mesh, display) {

}

std::future<void> MeshUserOperator::async() {
    if (mThread) {
        mThread->join();
        end_thread_control();
        prm = std::promise<void>();
    }
    begin_thread_control();
    mThread = std::make_unique<std::thread>([this] {
        try
        {
            (*this)();
            this->prm.set_value();
            end_thread_control();
        }
        catch (...)
        {
            restore_semaphore();
            end_thread_control();
            this->prm.set_exception(std::current_exception());
        }
        });
    return this->prm.get_future();
}

void MeshOperator::setDisplay(MeshDisplay* display) {
    this->display = display;
}