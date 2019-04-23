#include "zs-math.h"
#include "../../triggers.h"
#include "../../Misc/headers/zs_types.h"
#include <string>

#define NO_INDICES 0

#ifndef zs_mesh
#define zs_mesh

namespace ZSPIRE {

	class Mesh {
	public:
		bool alive; //Non destroyed
        std::string mesh_label;

		unsigned int meshVAO;
		unsigned int meshVBO;
		unsigned int meshEBO;

		unsigned int vertices_num;
		unsigned int indices_num;

        void Init();
        void setMeshData(ZSVERTEX* vertices, unsigned int* indices, unsigned int vertices_num, unsigned int indices_num);
        void setMeshData(ZSVERTEX* vertices, unsigned int vertices_num);
        void Draw();
        void DrawLines();
        void Destroy();

		Mesh();
	};

    void setupDefaultMeshes();
    void freeDefaultMeshes();

	Mesh* getPlaneMesh2D();
	Mesh* getUiSpriteMesh2D();
	Mesh* getIsoTileMesh2D();
    Mesh* getCubeMesh3D();
   // Mesh LoadMeshesFromFile(const char* file_path);

}


#endif
