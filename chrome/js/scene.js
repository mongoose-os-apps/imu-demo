"use strict";

var Scene = {
  canvas:      0,
  renderer:    0,
  scene:       0,
  camera:      0,
  hemiLight:   0,
  cube:        0,

  addCamera: function() {
    this.camera = new THREE.PerspectiveCamera(30, canvas.scrollWidth/canvas.scrollHeight, 0.1, 1000);
    this.camera.position.x = 1.4;
    this.camera.position.y = -1.5;
    this.camera.position.z = 1.5;
    this.camera.up = new THREE.Vector3(0, 0, 1);
    this.camera.lookAt(new THREE.Vector3(0.2, 0.2, 0.2));
  },

  addLights: function() {
    this.hemiLight = new THREE.HemisphereLight(0xffffff, 0xffffff, 0.6);
    this.hemiLight.color.setHSL(0.6, 1, 0.6);
    this.hemiLight.groundColor.setHSL(0.095, 1, 0.75);
    this.hemiLight.position.set(0, 100, 0);
    this.scene.add(this.hemiLight);
  },

  addCube: function() {
    var material = new THREE.MeshPhongMaterial({color: 0xffff00,
      specular: 0xa0a0a0,
      shininess: 15,
      vertexColors: THREE.FaceColors,
      shading: THREE.FlatShading
      });

    var materials = [
      new THREE.MeshLambertMaterial({
          map: THREE.ImageUtils.loadTexture('img/textures/Right.png')
      }),
      new THREE.MeshLambertMaterial({
          map: THREE.ImageUtils.loadTexture('img/textures/Left.png')
      }),
      new THREE.MeshLambertMaterial({
          map: THREE.ImageUtils.loadTexture('img/textures/Back.png')
      }),
      new THREE.MeshLambertMaterial({
          map: THREE.ImageUtils.loadTexture('img/textures/Front.png')
      }),
      new THREE.MeshLambertMaterial({
          map: THREE.ImageUtils.loadTexture('img/textures/Top.png')
      }),
      new THREE.MeshLambertMaterial({
          map: THREE.ImageUtils.loadTexture('img/textures/Bottom.png')
      }),
    ];
    material = new THREE.MeshFaceMaterial(materials);
    this.cube = new THREE.Mesh(new THREE.BoxGeometry(1.0, 0.6, 0.3), material);
    this.cube.position.x = 0.2;
    this.cube.position.y = 0.2;
    this.cube.position.z = 0.2;
    this.scene.add(this.cube);
  },

  init: function(dom_elem) {
    this.canvas = document.getElementById(dom_elem);
    this.renderer = new THREE.WebGLRenderer({ canvas: this.canvas });
    this.renderer.setSize(this.canvas.scrollWidth, this.canvas.scrollHeight);
    this.scene = new THREE.Scene();

    this.addCamera();
    this.addLights();
    this.addCube();
  },
};
