using UnityEngine;
using System.Collections;

public class enableWireframe : MonoBehaviour {

	private bool m_wireEnabled = false;
	public Canvas canvas;
	public rotateTeapot teapotScript;

	IEnumerator Start() {
		yield return new WaitForSeconds(6);
		m_wireEnabled = true;
		canvas.GetComponent<Canvas>().enabled = true;
		teapotScript.leftOffset = 0.5f;
	}

	void OnPreRender() {
		GL.wireframe = m_wireEnabled;
	}
	void OnPostRender() {
		GL.wireframe = !m_wireEnabled;
	}
}
