#version 410 core

out vec4 FragColor;
uniform vec4 ourColor;

in vec4 vCol;
void main()
{
   FragColor = vCol;//vec4(1.0f,1.0f,0.0f,1.0f);
}