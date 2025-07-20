#include "RenderObject.h"

void RenderObject::ApplyMeshModel(std::unique_ptr<Model> model)
{
    _model = std::move(model);
}
