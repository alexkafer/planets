#include "vert_buffer.hpp"

// Standard Headers
#include <iostream>
#include <memory>
#include <string>
#include <limits>

// Local Headers
#include "vert_attributes.hpp"
#include "gl_error.hpp"

GLuint VertBuffer::currentlyBoundVao = 0;

VertBuffer *VertBuffer::with(VertAttributes &attributes)
{
    return new VertBuffer(attributes);
}

void VertBuffer::uploadSingleMesh(SharedMesh mesh)
{
    with(mesh->attributes)->add(mesh)->upload();
}

VertBuffer::VertBuffer(VertAttributes &attributes)
: vertSize(attributes.getVertSize()), attrs(attributes)
{
    glGenVertexArrays(1, &vaoId);
}

void VertBuffer::bind()
{
    if (currentlyBoundVao != vaoId) // todo: this will break when glBindVertexArray() is called from another place.
    {
        glBindVertexArray(vaoId);
        currentlyBoundVao = vaoId;
        // vbo and ibo should still be bound at this point.
    }
}


VertBuffer *VertBuffer::add(SharedMesh mesh)
{
    if (mesh->vertBuffer)
        throw mesh->name + " was already added to a VertBuffer";

    // #ifdef EMSCRIPTEN
    // if (next || nrOfVerts + mesh->nrOfVertices > std::numeric_limits<GLushort>::max())
    // {
    //     if (!next) next = VertBuffer::with(attrs);
    //     next->add(mesh);
    //     return this;
    // }
    // #endif

    // std::cout << "Adding " << mesh->name << " to VertBuffer " << vaoId << "\n";
    meshes.push_back(mesh);

    mesh->baseVertex = nrOfVerts;
    nrOfVerts += mesh->nrOfVertices;

    mesh->indicesBufferOffset = nrOfIndices * sizeof(GLushort);
    nrOfIndices += mesh->nrOfIndices;

    mesh->vertBuffer = this;
    return this;
}

void VertBuffer::upload()
{
    if (vboId)
        throw "VertBuffer already uploaded";

    // std::cout << "Uploading vbo\n";
    bind();

    glGenBuffers(1, &vboId);    // create VertexBuffer
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, nrOfVerts * vertSize, NULL, GL_STATIC_DRAW);

    glGenBuffers(1, &iboId);    // create IndexBuffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * nrOfIndices, NULL, GL_STATIC_DRAW);

    GLuint vertsOffset = 0, indicesOffset = 0;
    for (std::weak_ptr<Mesh> m : meshes)
    {
        if (m.expired())
            throw "Trying to upload a VertBuffer whose Meshes are already destroyed";

        SharedMesh mesh = m.lock();
        GLuint
            vertsSize = mesh->nrOfVertices * vertSize,
            indicesSize = mesh->nrOfIndices * sizeof(GLushort);

        auto &indices = mesh->indices;
        // #if EMSCRIPTEN
        // if (!disposeOfflineData) indices = *&indices;

        // for (auto &i : indices) i += mesh->baseVertex;
        // #endif

        glBufferSubData(GL_ARRAY_BUFFER, vertsOffset, vertsSize, mesh->vertices.data());
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indicesOffset, indicesSize, indices.data());

        // if (disposeOfflineData) mesh->disposeOfflineData();

        vertsOffset += vertsSize;
        indicesOffset += indicesSize;
    }
    setAttrPointersAndEnable(attrs);
    uploaded = true;
}

void VertBuffer::setAttrPointersAndEnable(VertAttributes &attrs, unsigned int divisor, unsigned int locationOffset)
{
    GLint offset = 0;
    for (unsigned int i = locationOffset; i < locationOffset + attrs.nrOfAttributes(); i++)
    {
        auto &attr = attrs.get(i - locationOffset);
        glDisableVertexAttribArray(i);
        switch (attr.type)
        {
            case GL_INT:
            case GL_UNSIGNED_INT:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:

                glVertexAttribIPointer(
                        i,                                    // location of attribute that can be used in vertex shaders. eg: 'layout(location = 0) in vec3 position'
                        attr.size,                            // size.
                        attr.type,                             // type
                        attrs.getVertSize(),                  // stride
                        (void *)(uintptr_t)offset             // offset
                );
                break;
            default:
                glVertexAttribPointer(
                        i,                                    // location of attribute that can be used in vertex shaders. eg: 'layout(location = 0) in vec3 position'
                        attr.size,                            // size.
                        attr.type,                             // type
                        attr.normalized ? GL_TRUE : GL_FALSE, // normalized?
                        attrs.getVertSize(),                  // stride
                        (void *)(uintptr_t)offset             // offset
                );
        }
        glEnableVertexAttribArray(i);
        offset += attr.byteSize;

        if (divisor)
        {
            #ifdef EMSCRIPTEN
            EM_ASM({
                gl.vertexAttribDivisor($0, $1);
            }, i, divisor);
            #else
            glVertexAttribDivisor(i, divisor);
            #endif
        }
    }
}

GLuint VertBuffer::uploadPerInstanceData(VertData data, GLuint advanceRate)
{
    bind();
    GLuint id = instanceVbos.size();
    for (unsigned int i = 0; i < instanceVbos.size(); i++) if (instanceVbos[i] == (GLuint) -1) id = i;
    if (id == instanceVbos.size())
    {
        instanceVbos.emplace_back();
        instanceVboAttrs.emplace_back();
    }
    glGenBuffers(1, &instanceVbos[id]);
    instanceVboAttrs[id] = data.attributes;

    glBindBuffer(GL_ARRAY_BUFFER, instanceVbos[id]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.vertices.size(), &data.vertices[0], GL_STATIC_DRAW);
    usePerInstanceData(id, advanceRate);
    return id;
}

void VertBuffer::onMeshDestroyed()
{
    std::cout << "A mesh in this VB was destroyed\n";
    if (!inUse()) delete this;
}

bool VertBuffer::isUploaded() const
{
    return uploaded;
}

bool VertBuffer::inUse() const
{
    for (std::weak_ptr<Mesh> m : meshes)
        if (!m.expired()) return true;
    return false;
}

VertBuffer::~VertBuffer()
{
    glDeleteVertexArrays(1, &vaoId);
    glDeleteBuffers(1, &vboId);
    glDeleteBuffers(1, &iboId);

    for (auto &id : instanceVbos)
        if (id != (GLuint) -1) glDeleteBuffers(1, &id);

    if (vaoId == currentlyBoundVao)
        currentlyBoundVao = 0;
}

void VertBuffer::usePerInstanceData(GLuint instanceDataId, GLuint advanceRate)
{
    bind();
    glBindBuffer(GL_ARRAY_BUFFER, instanceVbos[instanceDataId]);
    setAttrPointersAndEnable(instanceVboAttrs[instanceDataId], advanceRate, attrs.nrOfAttributes());
}

void VertBuffer::deletePerInstanceData(GLuint instanceDataId)
{
    bind();
    glDeleteBuffers(1, &instanceVbos[instanceDataId]);
    instanceVbos[instanceDataId] = -1;
}
