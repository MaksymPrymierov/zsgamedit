#include "headers/zs3m-master.h"
#include <fstream>
#include <iostream>

ZS3M::SceneFileExport::SceneFileExport(){

}
void ZS3M::SceneFileExport::pushMesh(ZSPIRE::Mesh* mesh){
    this->meshes_toWrite.push_back(mesh);
}
void ZS3M::SceneFileExport::setRootNode(MeshNode* node){
    this->rootNode = node;
}
void ZS3M::SceneFileExport::write(std::string output_file){
    std::cout << "Writing ZS3M scene " << output_file << std::endl;
    std::ofstream stream;
    stream.open(output_file.c_str(), std::ofstream::binary);

    stream << "zs3mscene\n";
    unsigned int model_ver = 1000;
    unsigned int meshes_num = static_cast<unsigned int>(this->meshes_toWrite.size());
    unsigned int nodes_num = 0;
    getNodesNum(&nodes_num, this->rootNode);

    stream.write(reinterpret_cast<char*>(&model_ver), sizeof (unsigned int));
    stream.write(reinterpret_cast<char*>(&meshes_num), sizeof (unsigned int));
    stream.write(reinterpret_cast<char*>(&nodes_num), sizeof (unsigned int));
    //Write all nodes
    writeNode(&stream, rootNode);

    //Iterate over all meshes
    for(unsigned int mesh_i = 0; mesh_i < meshes_num; mesh_i ++){
        ZSPIRE::Mesh* mesh_ptr = this->meshes_toWrite[mesh_i];
        stream << "_MESH " << mesh_ptr->mesh_label << "\n";

        unsigned int vertexNum = mesh_ptr->vertices_num;
        unsigned int indexNum = mesh_ptr->indices_num;
        unsigned int bonesNum = static_cast<unsigned int>(mesh_ptr->bones.size());
        //Write base numbers
        stream.write(reinterpret_cast<char*>(&vertexNum), sizeof (unsigned int));
        stream.write(reinterpret_cast<char*>(&indexNum), sizeof (unsigned int));
        stream.write(reinterpret_cast<char*>(&bonesNum), sizeof (unsigned int));
        stream << "\n"; //Write divider
        //Write all vertices
        for (unsigned int v_i = 0; v_i < vertexNum; v_i ++) {
            ZSVERTEX* v_ptr = &mesh_ptr->vertices_arr[v_i];
            //Write vertex vectors
            stream.write(reinterpret_cast<char*>(&v_ptr->pos), sizeof(float) * 3);
            stream.write(reinterpret_cast<char*>(&v_ptr->uv), sizeof(float) * 2);
            stream.write(reinterpret_cast<char*>(&v_ptr->normal), sizeof(float) * 3);
            stream.write(reinterpret_cast<char*>(&v_ptr->tangent), sizeof(float) * 3);
            stream.write(reinterpret_cast<char*>(&v_ptr->bitangent), sizeof(float) * 3);
            stream.write(reinterpret_cast<char*>(&v_ptr->bones_num), sizeof(unsigned int));
            for(unsigned int vb_i = 0; vb_i < v_ptr->bones_num; vb_i ++){
                unsigned int bone_id = v_ptr->ids[vb_i];
                float b_weight = v_ptr->weights[vb_i];
                //Write bone values
                stream.write(reinterpret_cast<char*>(&bone_id), sizeof(unsigned int));
                stream.write(reinterpret_cast<char*>(&b_weight), sizeof(float));
            }
            stream << "\n"; //Write divider
        }
        for(unsigned int ind_i = 0; ind_i < indexNum; ind_i ++){
            stream.write(reinterpret_cast<char*>(&mesh_ptr->indices_arr[ind_i]), sizeof(unsigned int));
        }
        stream << "\n"; //Write divider
        for (unsigned int b_i = 0; b_i < bonesNum; b_i ++) {
            ZSPIRE::Bone* bone = &mesh_ptr->bones[b_i];
            //Write bone name
            stream << "b " << bone->bone_name << "\n";
            //Write offset matrix
            for(unsigned int m_i = 0; m_i < 4; m_i ++){
                for(unsigned int m_j = 0; m_j < 4; m_j ++){
                    float m_v = bone->offset.m[m_i][m_j];
                    stream.write(reinterpret_cast<char*>(&m_v), sizeof(float));
                }
            }

            stream << "\n"; //Write divider
        }

    }
    stream.close();
}

void ZS3M::SceneFileExport::writeNode(std::ofstream *stream, MeshNode* node){
    //Write node header
    *stream << "N " << node->node_label << " " << node->mesh_names.size() << " " << node->children.size() << " " << "\n";
    for(unsigned int mesh_i = 0; mesh_i < node->mesh_names.size(); mesh_i ++){
        //Write mesh name
        *stream << node->mesh_names[mesh_i] << "\n";
    }
    for(unsigned int ch_i = 0; ch_i < node->children.size(); ch_i ++){
        *stream << node->children[ch_i].node_label << "\n";
    }
    //Write node base matrix
    for(unsigned int m_i = 0; m_i < 4; m_i ++){
        for(unsigned int m_j = 0; m_j < 4; m_j ++){
            float m_v = node->node_transform.m[m_i][m_j];
            stream->write(reinterpret_cast<char*>(&m_v), sizeof(float));
        }
    }
    //Write all children
    for(unsigned int ch_i = 0; ch_i < node->children.size(); ch_i ++){
        writeNode(stream, &node->children[ch_i]);
    }
}

void ZS3M::SceneFileExport::getNodesNum(unsigned int* nds_ptr, MeshNode* node){
    *nds_ptr += 1;
    //Write all children
    for(unsigned int ch_i = 0; ch_i < node->children.size(); ch_i ++){
        getNodesNum(nds_ptr, &node->children[ch_i]);
    }
}


void ZS3M::ImportedSceneFile::loadFromFile(std::string output_file){
    std::cout << "Reading ZS3M scene " << output_file << std::endl;
    std::ifstream stream;
    stream.open(output_file.c_str(), std::ifstream::binary);

    std::string model_header;
    stream >> model_header;

    if(model_header.compare("zs3mscene") == true){
        stream.close();
        return;
    }

    unsigned int model_ver = 0;
    unsigned int meshes_num = 0;
    unsigned int nodes_num = 0;

    stream.read(reinterpret_cast<char*>(&model_ver), sizeof (unsigned int));

    if(model_ver != 1000){
        stream.close();
        return;
    }

    stream.read(reinterpret_cast<char*>(&meshes_num), sizeof (unsigned int));
    stream.read(reinterpret_cast<char*>(&nodes_num), sizeof (unsigned int));

    std::string prefix;
    while(!stream.eof()){
        stream >> prefix;

        if(prefix.compare("_MESH") == false){
            std::string mesh_label;
            stream >> mesh_label; // read mesh label

            unsigned int vertexNum = 0;
            unsigned int indexNum = 0;
            unsigned int bonesNum = 0;
            //Write base numbers
            stream.read(reinterpret_cast<char*>(&vertexNum), sizeof (unsigned int));
            stream.read(reinterpret_cast<char*>(&indexNum), sizeof (unsigned int));
            stream.read(reinterpret_cast<char*>(&bonesNum), sizeof (unsigned int));
        }
    }

    stream.close();
}
ZS3M::ImportedSceneFile::ImportedSceneFile(){

}
