using UnityEngine;
using System.Collections;

public class changeScene : MonoBehaviour {

	public int levelIndex;

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
		if(Input.GetKeyUp(KeyCode.RightArrow)) {
			// Next level
			Application.LoadLevelAsync(levelIndex);
		}
	}
}
