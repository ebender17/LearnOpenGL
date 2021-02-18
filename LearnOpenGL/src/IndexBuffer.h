#pragma once
class IndexBuffer
{
private: 
	unsigned int m_RendererID;
	unsigned int m_Count; 
public: 
	/* count means element count
	* ex. drawing square requires 6 vertices (count) 
	* not 24 (size in bytes)
	*/
	IndexBuffer(const unsigned int* data, unsigned int count); 
	~IndexBuffer(); 

	void Bind() const;
	void UnBind() const;

	inline unsigned int GetCount() const { return m_Count; }
};

