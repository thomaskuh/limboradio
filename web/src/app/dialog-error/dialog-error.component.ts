import {Component, Inject} from '@angular/core';
import {MAT_DIALOG_DATA, MatDialogRef} from '@angular/material';
import {RadioError} from '../client/radio-error';

@Component({
  selector: 'app-dialog-error',
  templateUrl: './dialog-error.component.html',
  styleUrls: ['./dialog-error.component.scss']
})
export class DialogErrorComponent {

  constructor(public dialogRef: MatDialogRef<DialogErrorComponent>, @Inject(MAT_DIALOG_DATA) public data: RadioError) { }

  close() {
    this.dialogRef.close();
  }

}
