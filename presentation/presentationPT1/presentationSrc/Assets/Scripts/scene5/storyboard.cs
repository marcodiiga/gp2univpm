using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class storyboard : MonoBehaviour {

	private int phase = 0;
	public GameObject kernel;
	private Color kernelFinalColor;
	public GameObject kernelText;
	private Color kernelTextFinalColor;
	public GameObject blurredImage;
	public GameObject convolutionText;

	// Use this for initialization
	void Start () {
		kernelFinalColor = kernel.GetComponent<Image>().color;
		kernelFinalColor.a = 255;
		kernelTextFinalColor = kernelText.GetComponent<Text>().color;
		kernelTextFinalColor.a = 255;
	}
	
	// Update is called once per frame
	void Update () {
	
		if (Input.GetKeyDown(KeyCode.Space))
			phase++;

		switch(phase) {
		case 1: {
				// Show kernel
				FadeToOpaque();
			}break;
		case 2: {
				// Hide text and start spline-moving
				kernelText.SetActive (false);
				convolutionText.SetActive(true);
				moveKernelAlongImage();
			}break;
		
		}

	}

	void FadeToOpaque () {
		// Lerp the colour of the texture between transparent and opaque
		kernel.GetComponent<Image>().color = Color.Lerp(kernel.GetComponent<Image>().color, kernelFinalColor, 0.003f * Time.deltaTime);
		kernelText.GetComponent<Text>().color = Color.Lerp(kernelText.GetComponent<Text>().color, kernelTextFinalColor, 0.003f * Time.deltaTime);
	}

	private int row = 11;
	private bool firstTime = true;
	void moveKernelAlongImage() {
		if (kernel.transform.localPosition.y > 170)
			return;

		if (firstTime) {
			kernel.transform.localPosition = new Vector3(-255, 220 - (50 * row), 0);
			firstTime = false;
		}

		// -255 to 255
		// 327 to 170
		Vector3 newPos = new Vector3(-255, 220 - (50 * row), 0);
		newPos.x = kernel.transform.localPosition.x + 440.0f*Time.deltaTime;

		if (newPos.x > 252) {
			newPos.x = -255;
			row--;
			newPos.y = 220 - (50 * row);
			if (row == 0)
				blurredImage.GetComponent<RectTransform>().sizeDelta = new Vector2(blurredImage.GetComponent<RectTransform>().sizeDelta.x, 
				                                                                   512);
			else if (row < 10)
				blurredImage.GetComponent<RectTransform>().sizeDelta = new Vector2(blurredImage.GetComponent<RectTransform>().sizeDelta.x, 
			                                                                   blurredImage.GetComponent<RectTransform>().sizeDelta.y + 50);
		}
		kernel.transform.localPosition = newPos;
	}
}
