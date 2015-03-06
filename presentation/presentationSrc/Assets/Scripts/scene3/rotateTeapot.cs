using UnityEngine;
using System.Collections;

public class rotateTeapot : MonoBehaviour {

	public float leftOffset = 0;
	private float myOffset = 0;
	private Vector3 initialPos;

	void Start() {
		initialPos = gameObject.transform.position;
	}

	void Update () {
		Vector3 angles = gameObject.transform.eulerAngles;
		angles.y += Time.deltaTime * 20.0f;
		gameObject.transform.eulerAngles = angles;
		if (leftOffset > myOffset) {
			myOffset = Mathf.Lerp(myOffset, leftOffset, Time.deltaTime);
			Vector3 pos = initialPos;
			pos.x -= myOffset;
			gameObject.transform.position = pos;
		}
	}
}
