using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using SharpGL;
using SharpGL.Shaders;
using SharpGL.Enumerations;
using SharpGL.VertexBuffers;

namespace AudioScope
{
    public partial class FormAudioScope : Form
    {
        private const string VERTEX_SHADER_PATH = "shaders/line/line.vert";
        private const string FRAGMENT_SHADER_PATH = "shaders/line/line.frag";
        private const string GEOMETRY_SHADER_PATH = "shaders/line/line.geom";

        private AudioScope _audioScope = new AudioScope();
        private SharpGL.Shaders.ShaderProgram _prog;
        private List<float[]> _dump;
        private int _dumpIndex = 0;

        public FormAudioScope()
        {
            InitializeComponent();
        }

        private void FormAudioScope_Load(object sender, EventArgs e)
        {
            _audioScope.InitAudio();
            _audioScope.Start();

            _dump = new List<float[]>();

            using (StreamReader sr = new StreamReader("dump.txt"))
            {
                while (!sr.EndOfStream)
                {
                    var line = sr.ReadLine();
                    var fields = line.Split(',');
                    List<float> samples = new List<float>();
                    foreach (var field in fields)
                    {
                        if (!String.IsNullOrEmpty(field.Trim()))
                        {
                            samples.Add(float.Parse(field));
                        }
                    }
                    _dump.Add(samples.ToArray());
                }
            }
        }

        private void OpenGLControl_OpenGLInitialized(object sender, EventArgs e)
        {
            OpenGL gl = this.openGLControl1.OpenGL;

            string fragmentShaderCode = null;
            using (StreamReader sr = new StreamReader(FRAGMENT_SHADER_PATH))
            {
                fragmentShaderCode = sr.ReadToEnd();
            }

            string vertexShaderCode = null;
            using (StreamReader sr = new StreamReader(VERTEX_SHADER_PATH))
            {
                vertexShaderCode = sr.ReadToEnd();
            }

            _prog = new SharpGL.Shaders.ShaderProgram();
            _prog.Create(gl, vertexShaderCode, fragmentShaderCode, null);

            string geometryShaderCode = null;
            if (File.Exists(GEOMETRY_SHADER_PATH))
            {
                using (StreamReader sr = new StreamReader(GEOMETRY_SHADER_PATH))
                {
                    geometryShaderCode = sr.ReadToEnd();
                }

                Shader geometryShader = new Shader();
                geometryShader.Create(gl, OpenGL.GL_GEOMETRY_SHADER, geometryShaderCode);
                gl.AttachShader(_prog.ShaderProgramObject, geometryShader.ShaderObject);
            }

            gl.LinkProgram(_prog.ShaderProgramObject);

            // Now that we've compiled and linked the shader, check it's link status.If it's not linked properly, we're
            //  going to throw an exception.
            if (_prog.GetLinkStatus(gl) == false)
            {
                throw new SharpGL.Shaders.ShaderCompilationException(string.Format("Failed to link shader program with ID {0}.", _prog.ShaderProgramObject), _prog.GetInfoLog(gl));
            }
        }

        private void openGLControl1_OpenGLDraw(object sender, RenderEventArgs e)
        {
            //  Get the OpenGL object, just to clean up the code.
            OpenGL gl = this.openGLControl1.OpenGL;

            gl.Clear(OpenGL.GL_COLOR_BUFFER_BIT | OpenGL.GL_DEPTH_BUFFER_BIT);  // Clear The Screen And The Depth Buffer

            gl.UseProgram(_prog.ShaderProgramObject);

            int windowID = gl.GetUniformLocation(_prog.ShaderProgramObject, "window");
            int nID = gl.GetUniformLocation(_prog.ShaderProgramObject, "n");
            int baseHueID = gl.GetUniformLocation(_prog.ShaderProgramObject, "base_hue");
            int colorizeID = gl.GetUniformLocation(_prog.ShaderProgramObject, "colorize");
            int thicknessID = gl.GetUniformLocation(_prog.ShaderProgramObject, "thickness");
            int minThicknessID = gl.GetUniformLocation(_prog.ShaderProgramObject, "min_thickness");
            int thinningID = gl.GetUniformLocation(_prog.ShaderProgramObject, "thinning");
            int decayID = gl.GetUniformLocation(_prog.ShaderProgramObject, "decay");
            int desaturationID = gl.GetUniformLocation(_prog.ShaderProgramObject, "desaturation");

            gl.Uniform2(windowID, 1024.0f, 768.0f);
            gl.Uniform1(nID, 5);
            gl.Uniform1(thicknessID, 10.0f);
            gl.Uniform1(minThicknessID, 1.5f);
            gl.Uniform1(thinningID, 0.05f);
            gl.Uniform1(baseHueID, 0.0f);
            gl.Uniform1(colorizeID, 1);
            gl.Uniform1(decayID, 0.3f);
            gl.Uniform1(desaturationID, 0.1f);

            //var data = _audioScope.GetSample();
            var data = _dump[_dumpIndex++];
            if (_dumpIndex >= _dump.Count)
            {
                _dumpIndex = 0;
            }

            if (data != null)
            {
                //var dataFlattened = data.SelectMany(x => x.Vec).ToArray();
                //float[] dataFlattened 

                VertexBuffer vertexBuffer = new VertexBuffer();
                vertexBuffer.Create(gl);
                vertexBuffer.Bind(gl);
                vertexBuffer.SetData(gl, 0, data, false, 4);

                gl.DrawArrays(OpenGL.GL_LINE_STRIP_ADJACENCY, 0, data.Length);
            }
        }
    }
}