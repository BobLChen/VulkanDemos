#pragma once

#include "Common/Common.h"
#include "Common/Log.h"
#include "Graphics/Material/Material.h"
#include "Renderable.h"

#include <memory>

class Mesh
{
public:
    typedef std::shared_ptr<Renderable> RenderablePtr;
    typedef std::shared_ptr<Material>   MaterialPtr;
    typedef std::vector<RenderablePtr>  RenderablesArray;
    typedef std::vector<MaterialPtr>    MaterialsArray;
    
public:
    Mesh()
        : m_Count(0)
    {
        
    }
    
    virtual ~Mesh()
    {
        
    }
    
    FORCEINLINE void AddSubMesh(RenderablePtr renderable, MaterialPtr material)
    {
        m_Count += 1;
        m_Materials.push_back(material);
        m_Renderables.push_back(renderable);
    }
    
    FORCEINLINE void SetSubMesh(int32 index, RenderablePtr renderable, MaterialPtr material)
    {
        if (index < m_Count)
        {
            m_Materials[index] = material;
            m_Renderables[index] = renderable;
        }
        else
        {
            m_Count += 1;
            m_Materials.push_back(material);
            m_Renderables.push_back(renderable);
        }
    }
    
    FORCEINLINE const RenderablesArray& GetRenderables() const
    {
        return m_Renderables;
    }
    
    FORCEINLINE const MaterialsArray& GetMaterials() const
    {
        return m_Materials;
    }
    
    FORCEINLINE int32 GetSubMeshNum() const
    {
        return m_Count;
    }
    
protected:
    int32               m_Count;
    RenderablesArray    m_Renderables;
    MaterialsArray      m_Materials;
};
