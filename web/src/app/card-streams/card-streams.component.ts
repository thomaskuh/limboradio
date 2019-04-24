import {Component, Input, OnInit} from '@angular/core';
import {ClientService} from '../client/client.service';
import {RadioStream} from '../client/radio-stream';
import {MatDialog, MatSnackBar} from '@angular/material';
import {DialogStreamComponent} from '../dialog-stream/dialog-stream.component';
import {SnackbarMessageComponent} from '../snackbar-message/snackbar-message.component';
import {RadioRequest} from '../client/radio-request';

@Component({
  selector: 'app-card-streams',
  templateUrl: './card-streams.component.html',
  styleUrls: ['./card-streams.component.scss']
})
export class CardStreamsComponent implements OnInit {

  /**
   * Selected/Active stream idx.
   */
  @Input()
  active = 0;

  items: RadioStream[] = [];
  disabled = false;

  constructor(private client: ClientService, private dialog: MatDialog, private snackBar: MatSnackBar) { }

  ngOnInit() {
    this.load();
  }

  load() {
    this.client.getStreams().subscribe(x => this.items = x.streams);
  }

  save() {
    if(this.disabled) return;
    this.disabled = true;

    const req = new RadioRequest();
    req.streams = this.items;
    this.client.saveThatThang(req).subscribe(
      result => {
        this.snackBar.openFromComponent(SnackbarMessageComponent, {duration: 3000, data: 'DONE'});
        this.disabled = false;
      },
      error => {
        this.disabled = false;
      }
    );
  }

  setStream(idx: number): void {
    const req = new RadioRequest();
    req.stream = idx;
    this.client.saveThatThang(req).subscribe();
  }

  add(): void {
    let dialogRef = this.dialog.open(DialogStreamComponent, {data: null}).afterClosed().subscribe(result => {
      if(result) this.items.push(result);
    });
  }

  delete(idx: number): void {
    this.items.splice(idx, 1);
  }

  up(idx: number): void {
    if(idx > 0) this.swap(idx, idx - 1);
  }

  down(idx: number): void {
    if(idx < this.items.length - 1) this.swap(idx, idx + 1);
  }

  edit(item: RadioStream): void {
    this.dialog.open(DialogStreamComponent, {data: item});
  }

  swap(idxA: number, idxB: number): void {
    const tmp = this.items[idxA];
    this.items[idxA] = this.items[idxB];
    this.items[idxB] = tmp;
  }

}
