#include <iostream>
#include <fstream>
#include "SubDivMesh.hpp"
#include "Debug.hpp"

SubDivMesh::SubDivMesh(const std::string &fname) {
	e.clear(); n.clear();
	std::ifstream f(fname);
	if (!f.is_open()) return;
	int nv;
	f>>nv;
	float x,y,z;
	for (int i=0;i<nv;i++) {
		f>>x>>y>>z;
		n.push_back(Nodo({x,y,z}));
	}
	int ne;
	f>>ne;
	int v0,v1,v2,v3;
	for (int i=0;i<ne;i++) {
		f>>nv>>v0>>v1>>v2;
		if (nv==3) { agregarElemento(v0,v1,v2); }
		else { f>>v3; agregarElemento(v0,v1,v2,v3); }
	}
	f.close();
	makeVecinos();
	
}

void SubDivMesh::agregarElemento(int n0, int n1, int n2, int n3) {
	cg_assert(n0>=0 and n0<static_cast<int>(n.size()),"El indice n0 no es valido");
	cg_assert(n1>=0 and n1<static_cast<int>(n.size()),"El indice n1 no es valido");
	cg_assert(n2>=0 and n2<static_cast<int>(n.size()),"El indice n2 no es valido");
	cg_assert(n3>=-1 and n3<static_cast<int>(n.size()),"El indice n3 no es valido");
	cg_assert(n1!=n2 and n1!=n3 and n1!=n0 and n2!=n3 and n2!=n0 and n3!=n0,"Vertice repetido");
	int ie=e.size(); e.push_back(Elemento(n0,n1,n2,n3)); // agrega el Elemento
	// avisa a cada nodo que ahora es vertice de este elemento
	n[n0].e.push_back(ie); n[n1].e.push_back(ie);
	n[n2].e.push_back(ie); if (n3>=0) n[n3].e.push_back(ie);
	
}

void SubDivMesh::reemplazarElemento(int ie, int n0, int n1, int n2, int n3) {
	cg_assert(ie>=0 and ie<static_cast<int>(e.size()),"El indice de elemento no es valido");
	cg_assert(n0>=0 and n0<static_cast<int>(n.size()),"El indice n0 no es valido");
	cg_assert(n1>=0 and n1<static_cast<int>(n.size()),"El indice n1 no es valido");
	cg_assert(n2>=0 and n2<static_cast<int>(n.size()),"El indice n2 no es valido");
	cg_assert(n3>=-1 and n3<static_cast<int>(n.size()),"El indice n3 no es valido");
	cg_assert(n1!=n2 and n1!=n3 and n1!=n0 and n2!=n3 and n2!=n0 and n3!=n0,"Vertice repetido");
	Elemento &ei=e[ie];
	// estos nodos ya no seran vertices de este elemento
	for (int i=0;i<ei.nv;i++) {
		std::vector<int> &ve=n[ei[i]].e;
		ve.erase(find(ve.begin(),ve.end(),ie));
	}
	ei.SetNodos(n0,n1,n2,n3);
	// estos nodos ahora son vertices
	n[n0].e.push_back(ie); n[n1].e.push_back(ie); n[n2].e.push_back(ie); 
	if (n3>=0) n[n3].e.push_back(ie); 
}

// Identifica los pares de elementos vecinos y las aristas de frontera
// Actualiza el atributo v (lista de vecinos) de cada elemento y el atributo es_frontera de cada nodo
void SubDivMesh::makeVecinos() {
	// inicializa
	for (size_t i=0;i<n.size();i++) n[i].es_frontera=false; // le dice a todos los nodos que son frontera
	for (size_t i=0;i<e.size();i++) e[i].v[0]=e[i].v[1]=e[i].v[2]=e[i].v[3]=-1; // le dice a todos los elementos que no tienen vecinos
	// identificacion de vecinos
	for (size_t ie=0;ie<e.size();ie++) { // por cada elemento
		for (int j=0;j<e[ie].nv;j++) { // por cada arista
			if (e[ie].v[j]>=0) continue; // ya se hizo
			int in0=e[ie][j], in1=e[ie][j+1]; // 1er y 2do nodo de la arista
			bool arista_frontera = true;
			for (size_t k=0;k<n[in0].e.size();k++) { // recorro los elementos del primer nodo
				int iev=n[in0].e[k];
				if (iev==ie) continue; // es este mismo
				// se fija si tiene a in1 (el 2do nodo)
				int ix=e[iev].Indice(in1);
				if (ix<0) continue; 
				// tiene al 2do
				e[ie].v[j]=n[in0].e[k]; // ese es el vecino
				e[iev].v[ix]=ie;
				arista_frontera = false;
				break; // solo dos posibles vecinos para una arista
			}
			if (arista_frontera) 
				n[in0].es_frontera = n[in1].es_frontera = true;
		}
	}
}

void SubDivMesh::verificarIntegridad() const {
	int esz = e.size(), nsz = n.size();
	// ver para cada nodo de un elemento, quu el nodo tenga la ref al elemento en su e
	for (int ie=0; ie<esz; ++ie) { 
		const Elemento &ei = e[ie];
		for (int in : ei) {
			cg_assert(in>=0 and in<nsz,"El elemento ie tiene un indice de vertice no valido");
			auto &enj = n[in].e;
			cg_assert(std::find(enj.begin(),enj.end(),ie)!=enj.end(),"El nodo in no tiene en su .e al elemento ie");
		}
	}
	
	// ver que cada elemento al que un nodo dice pertenecer, efectivamente lo contenga
	for (int in=0; in<nsz; ++in) {
		const Nodo &ni = n[in];
		for (int ie : ni.e) {
			cg_assert(ie>=0 and ie<esz,"El nodo in tiene un indice de elemento no valido");
			cg_assert(e[ie].Tiene(in),"El elemento ie no tiene al nodo in entre en su .n");
		}
	}
}

