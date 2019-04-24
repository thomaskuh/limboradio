import {Component, Inject, OnInit} from '@angular/core';
import {MAT_DIALOG_DATA, MatDialogRef} from '@angular/material';
import {RadioStream} from '../client/radio-stream';

@Component({
  selector: 'app-dialog-stream',
  templateUrl: './dialog-stream.component.html',
  styleUrls: ['./dialog-stream.component.scss']
})
export class DialogStreamComponent implements OnInit {

  item = new RadioStream();
  create = true;  // Mode: Either "Create new" or "Edit existing"

  constructor(public dialogRef: MatDialogRef<DialogStreamComponent>, @Inject(MAT_DIALOG_DATA) public data: RadioStream) { }

  ngOnInit(): void {
    if(this.data) {
      this.item = this.data;
      this.create = false;
    }
  }

  cancel() {
    this.dialogRef.close();
  }

  ok() {
    if(this.create) {
      this.dialogRef.close(this.item);
    }
    else {
      this.dialogRef.close();
    }
  }


}
