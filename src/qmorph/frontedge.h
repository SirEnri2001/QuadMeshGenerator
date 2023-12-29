class Halfedge;
struct FrontEdge {
	const Halfedge* he;
	const Halfedge* top;
	FrontEdge* prevFe;
	FrontEdge* nextFe;
	bool needTop = false;
	bool rightCornerSharp = false;

	FrontEdge* getPrevFe() const {
		return prevFe;
	}

	FrontEdge* getNextFe() const {
		return nextFe;
	}

	const Halfedge* getTop() {
		return top;
	}

	void setTop(const Halfedge* top) {
		this->top = top;
	}

	bool isLeftCornerSharp() {
		return prevFe->rightCornerSharp;
	}

	bool isRightCornerSharp() {
		return rightCornerSharp;
	}
};