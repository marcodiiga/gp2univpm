using UnityEngine;
using System.Collections;

public class rotateLogo : MonoBehaviour {

	public GameObject logo;

	// Use this for initialization
	void Start () {
		QualitySettings.SetQualityLevel(5);
		Screen.showCursor = false;
	}
	
	// Update is called once per frame
	void Update () {
		Vector3 angles = logo.transform.eulerAngles;
		angles.y += 1.0f * Time.deltaTime * 20.0f;
		if(logo.transform.eulerAngles.y > 90.0f && logo.transform.eulerAngles.y < 180) {
			angles.y += 180;
		}
		logo.transform.eulerAngles = angles;
	}
}
