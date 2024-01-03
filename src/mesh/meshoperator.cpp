#include "meshoperator.h"

MeshOperator::MeshOperator(Mesh* mesh) :mesh(mesh)
{

}

void MeshOperator::create() {

}

MeshInteriorOperator::MeshInteriorOperator(Mesh* mesh) : MeshOperator(mesh) {

}

MeshUserOperator::MeshUserOperator(Mesh* mesh) : MeshOperator(mesh) {

}

std::future<void> MeshUserOperator::async() {
    mThread = std::make_unique<std::thread>([this] {
        try
        {
            (*this)();
            this->prm.set_value();
        }
        catch (...)
        {
            this->prm.set_exception(std::current_exception());
        }
        });
    return this->prm.get_future();
}