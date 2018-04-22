#ifndef CLASS_NCINE_GLSHADERATTRIBUTES
#define CLASS_NCINE_GLSHADERATTRIBUTES

#include "nctl/HashMap.h"
#include "GLVertexFormat.h"

namespace ncine {

class GLShaderProgram;
class GLBufferObject;

/// A class to handle all the attributes of a shader program using a hashmap
class GLShaderAttributes
{
  public:
	GLShaderAttributes();
	explicit GLShaderAttributes(GLShaderProgram *shaderProgram);
	void setProgram(GLShaderProgram *shaderProgram);

	GLVertexFormat::Attribute *attribute(const char *name);
	void defineVertexFormat(const GLBufferObject *vbo);

  private:
	GLShaderProgram *shaderProgram_;

	static GLVertexFormat::Attribute attributeNotFound_;
	nctl::StringHashMap<int> attributeLocations_;
	GLVertexFormat vertexFormat_;

	void importAttributes();
};

}

#endif
